// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_SCRIPT_IMPORT_MAP_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_SCRIPT_IMPORT_MAP_H_

#include "base/macros.h"
#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/platform/heap/handle.h"
#include "third_party/blink/renderer/platform/weborigin/kurl.h"
#include "third_party/blink/renderer/platform/weborigin/kurl_hash.h"
#include "third_party/blink/renderer/platform/wtf/hash_map.h"

namespace blink {

class ConsoleLogger;
class ParsedSpecifier;

// Import maps.
// https://wicg.github.io/import-maps/
// https://github.com/WICG/import-maps/blob/master/spec.md
class ImportMap final : public GarbageCollectedFinalized<ImportMap> {
 public:
  static ImportMap* Create(const String& text,
                           const KURL& base_url,
                           ConsoleLogger& logger);

  explicit ImportMap(const HashMap<String, Vector<KURL>>& imports)
      : imports_(imports) {}

  // Returns nullopt when not mapped by |this| import map (i.e. the import map
  // doesn't have corresponding keys).
  // Returns a null URL when resolution fails.
  base::Optional<KURL> Resolve(const ParsedSpecifier&,
                               String* debug_message) const;

  String ToString() const;

  void Trace(Visitor*) {}

 private:
  HashMap<String, Vector<KURL>> imports_;
};

}  // namespace blink

#endif
