// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_CONTAINED_SHELL_CONTAINED_SHELL_CONTROLLER_H_
#define ASH_CONTAINED_SHELL_CONTAINED_SHELL_CONTROLLER_H_

#include "ash/ash_export.h"
#include "ash/public/interfaces/contained_shell.mojom.h"
#include "base/macros.h"
#include "mojo/public/cpp/bindings/binding_set.h"

class PrefRegistrySimple;

namespace ash {

// ContainedShellController allows a consumer of ash to provide a
// ContainedShellClient, to which we dispatch requests.
// TODO(910226): remove this code once the ContainedShell demo is complete and
// no longer needed.
class ASH_EXPORT ContainedShellController
    : public mojom::ContainedShellController {
 public:
  ContainedShellController();
  ~ContainedShellController() override;

  // Register prefs related to the Contained Shell.
  static void RegisterProfilePrefs(PrefRegistrySimple* registry);

  // Binds the mojom::ContainedShellController interface to this object.
  void BindRequest(mojom::ContainedShellControllerRequest request);

  // Returns if the Contained Shell is enabled for the current user.
  bool IsEnabled();

  // Starts the ContainedShell feature by sending LaunchContainedShell
  // request to ContainedShellClient.
  void LaunchContainedShell();

  // mojom::ContainedShellController:
  void SetClient(mojom::ContainedShellClientPtr client) override;

 private:
  mojom::ContainedShellClientPtr contained_shell_client_;
  mojo::BindingSet<mojom::ContainedShellController> bindings_;

  DISALLOW_COPY_AND_ASSIGN(ContainedShellController);
};

}  // namespace ash

#endif  // ASH_CONTAINED_SHELL_CONTAINED_SHELL_CONTROLLER_H
