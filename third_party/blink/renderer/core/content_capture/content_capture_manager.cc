// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/core/content_capture/content_capture_manager.h"

#include "third_party/blink/renderer/core/content_capture/content_holder.h"
#include "third_party/blink/renderer/core/layout/layout_text.h"

namespace blink {

ContentCaptureManager::ContentCaptureManager(Document& document,
                                             NodeHolder::Type type)
    : document_(&document), node_holder_type_(type) {}

ContentCaptureManager::~ContentCaptureManager() = default;

NodeHolder ContentCaptureManager::GetNodeHolder(Node& node) {
  if (first_node_holder_created_) {
    ScheduleTask(ContentCaptureTask::ScheduleReason::kContentChange);
  } else {
    ScheduleTask(ContentCaptureTask::ScheduleReason::kFirstContentChange);
    first_node_holder_created_ = true;
  }
  if (node_holder_type_ == NodeHolder::Type::kID)
    return NodeHolder(DOMNodeIds::IdForNode(&node));
  return NodeHolder(base::MakeRefCounted<ContentHolder>(node));
}

void ContentCaptureManager::ScheduleTask(
    ContentCaptureTask::ScheduleReason reason) {
  if (!content_capture_idle_task_.get()) {
    content_capture_idle_task_ = CreateContentCaptureTask();
  }
  content_capture_idle_task_->Schedule(reason);
}

scoped_refptr<ContentCaptureTask>
ContentCaptureManager::CreateContentCaptureTask() {
  return base::MakeRefCounted<ContentCaptureTask>(*document_, *this);
}

void ContentCaptureManager::NotifyNodeDetached(const NodeHolder& node_holder) {
  if (node_holder.type == NodeHolder::Type::kID) {
    Node* node = DOMNodeIds::NodeForId(node_holder.id);
    if (node && HasSent(*node))
      content_capture_idle_task_->OnNodeDetached(*node);
  } else if (node_holder.type == NodeHolder::Type::kTextHolder) {
    ContentHolder* content_holder =
        static_cast<ContentHolder*>(node_holder.text_holder.get());
    if (!content_holder || !content_holder->IsValid() ||
        !content_holder->HasSent()) {
      return;
    }
    content_capture_idle_task_->OnNodeDetached(*(content_holder->GetNode()));
  }
}

void ContentCaptureManager::OnLayoutTextWillBeDestroyed(
    NodeHolder node_holder) {
  DCHECK(!node_holder.is_empty);
  NotifyNodeDetached(node_holder);
  if (node_holder.type == NodeHolder::Type::kTextHolder) {
    ContentHolder* content_holder =
        static_cast<ContentHolder*>(node_holder.text_holder.get());
    if (content_holder)
      content_holder->OnNodeDetachedFromLayoutTree();
  }
  ScheduleTask(ContentCaptureTask::ScheduleReason::kContentChange);
}

void ContentCaptureManager::OnScrollPositionChanged() {
  ScheduleTask(ContentCaptureTask::ScheduleReason::kScrolling);
}

bool ContentCaptureManager::HasSent(const Node& node) {
  return sent_nodes_.Contains(&node);
}

void ContentCaptureManager::OnSent(const Node& node) {
  sent_nodes_.insert(WeakMember<const Node>(&node));
}

void ContentCaptureManager::Trace(Visitor* visitor) {
  visitor->Trace(document_);
  visitor->Trace(sent_nodes_);
}

void ContentCaptureManager::Shutdown() {
  if (content_capture_idle_task_) {
    content_capture_idle_task_->Shutdown();
    content_capture_idle_task_.reset();
  }
}

}  // namespace blink
