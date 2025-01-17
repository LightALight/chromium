// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/controller/crash_memory_metrics_reporter_impl.h"

#include "base/allocator/partition_allocator/oom_callback.h"
#include "base/metrics/histogram_macros.h"
#include "base/process/memory.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "third_party/blink/public/platform/platform.h"
#include "third_party/blink/renderer/controller/memory_usage_monitor_android.h"
#include "third_party/blink/renderer/platform/bindings/v8_per_isolate_data.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/wtf/allocator/partitions.h"

namespace blink {

// static
void CrashMemoryMetricsReporterImpl::Bind(
    mojom::blink::CrashMemoryMetricsReporterRequest request) {
  // This should be called only once per process on RenderProcessWillLaunch.
  DCHECK(!CrashMemoryMetricsReporterImpl::Instance().binding_.is_bound());
  CrashMemoryMetricsReporterImpl::Instance().binding_.Bind(std::move(request));
}

CrashMemoryMetricsReporterImpl& CrashMemoryMetricsReporterImpl::Instance() {
  DEFINE_STATIC_LOCAL(CrashMemoryMetricsReporterImpl,
                      crash_memory_metrics_reporter_impl, ());
  return crash_memory_metrics_reporter_impl;
}

CrashMemoryMetricsReporterImpl::CrashMemoryMetricsReporterImpl()
    : binding_(this) {
  base::SetPartitionAllocOomCallback(
      CrashMemoryMetricsReporterImpl::OnOOMCallback);
}

CrashMemoryMetricsReporterImpl::~CrashMemoryMetricsReporterImpl() = default;

void CrashMemoryMetricsReporterImpl::SetSharedMemory(
    base::UnsafeSharedMemoryRegion shared_metrics_buffer) {
  // This method should be called only once per process.
  DCHECK(!shared_metrics_mapping_.IsValid());
  shared_metrics_mapping_ = shared_metrics_buffer.Map();
}

void CrashMemoryMetricsReporterImpl::OnMemoryPing(MemoryUsage usage) {
  WriteIntoSharedMemory(
      CrashMemoryMetricsReporterImpl::MemoryUsageToMetrics(usage));
}

void CrashMemoryMetricsReporterImpl::WriteIntoSharedMemory(
    const OomInterventionMetrics& metrics) {
  if (!shared_metrics_mapping_.IsValid())
    return;
  auto* metrics_shared =
      shared_metrics_mapping_.GetMemoryAs<OomInterventionMetrics>();
  memcpy(metrics_shared, &metrics, sizeof(OomInterventionMetrics));
}

void CrashMemoryMetricsReporterImpl::OnOOMCallback() {
  // TODO(yuzus: Support allocation failures on other threads as well.
  if (!IsMainThread())
    return;
  // If shared_metrics_mapping_ is not set, it means OnNoMemory happened before
  // initializing render process host sets the shared memory.
  if (!CrashMemoryMetricsReporterImpl::Instance()
           .shared_metrics_mapping_.IsValid())
    return;
  // Else, we can send the allocation_failed bool.
  OomInterventionMetrics metrics;
  // TODO(yuzus): Report this UMA on all the platforms. Currently this is only
  // reported on Android.
  metrics.allocation_failed = 1;  // true
  CrashMemoryMetricsReporterImpl::Instance().WriteIntoSharedMemory(metrics);
}

OomInterventionMetrics
CrashMemoryMetricsReporterImpl::GetCurrentMemoryMetrics() {
  return MemoryUsageToMetrics(
      MemoryUsageMonitor::Instance().GetCurrentMemoryUsage());
}

// static
OomInterventionMetrics CrashMemoryMetricsReporterImpl::MemoryUsageToMetrics(
    MemoryUsage usage) {
  OomInterventionMetrics metrics = {};

  DCHECK(!std::isnan(usage.private_footprint_bytes));
  DCHECK(!std::isnan(usage.swap_bytes));
  DCHECK(!std::isnan(usage.vm_size_bytes));
  metrics.current_blink_usage_kb =
      (usage.v8_bytes + usage.blink_gc_bytes + usage.partition_alloc_bytes) /
      1024;

  DCHECK(!std::isnan(usage.private_footprint_bytes));
  DCHECK(!std::isnan(usage.swap_bytes));
  DCHECK(!std::isnan(usage.vm_size_bytes));
  metrics.current_private_footprint_kb = usage.private_footprint_bytes / 1024;
  metrics.current_swap_kb = usage.swap_bytes / 1024;
  metrics.current_vm_size_kb = usage.vm_size_bytes / 1024;
  metrics.allocation_failed = 0;  // false
  return metrics;
}

}  // namespace blink
