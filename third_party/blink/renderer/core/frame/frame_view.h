// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_FRAME_FRAME_VIEW_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_FRAME_FRAME_VIEW_H_

#include "third_party/blink/renderer/core/frame/embedded_content_view.h"
#include "third_party/blink/renderer/platform/wtf/casting.h"

namespace blink {

struct IntrinsicSizingInfo;

class CORE_EXPORT FrameView : public EmbeddedContentView {
 public:
  ~FrameView() override = default;
  virtual void UpdateViewportIntersectionsForSubtree() = 0;

  virtual bool GetIntrinsicSizingInfo(IntrinsicSizingInfo&) const = 0;
  virtual bool HasIntrinsicSizingInfo() const = 0;

  bool IsFrameView() const override { return true; }
};

template <>
struct DowncastTraits<FrameView> {
  static bool AllowFrom(const EmbeddedContentView& embedded_content_view) {
    return embedded_content_view.IsFrameView();
  }
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_CORE_FRAME_FRAME_VIEW_H_
