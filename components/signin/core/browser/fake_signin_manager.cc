// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/signin/core/browser/fake_signin_manager.h"

#include "base/callback_helpers.h"
#include "build/build_config.h"
#include "components/prefs/pref_service.h"
#include "components/signin/core/browser/account_tracker_service.h"
#include "components/signin/core/browser/signin_client.h"
#include "components/signin/core/browser/signin_metrics.h"
#include "components/signin/core/browser/signin_pref_names.h"

#if !defined(OS_CHROMEOS)

FakeSigninManager::FakeSigninManager(
    SigninClient* client,
    ProfileOAuth2TokenService* token_service,
    AccountTrackerService* account_tracker_service,
    GaiaCookieManagerService* cookie_manager_service)
    : FakeSigninManager(client,
                        token_service,
                        account_tracker_service,
                        cookie_manager_service,
                        signin::AccountConsistencyMethod::kDisabled) {}

FakeSigninManager::FakeSigninManager(
    SigninClient* client,
    ProfileOAuth2TokenService* token_service,
    AccountTrackerService* account_tracker_service,
    GaiaCookieManagerService* cookie_manager_service,
    signin::AccountConsistencyMethod account_consistency)
    : SigninManager(client,
                    token_service,
                    account_tracker_service,
                    cookie_manager_service,
                    account_consistency) {}

FakeSigninManager::~FakeSigninManager() {}

void FakeSigninManager::SignIn(const std::string& gaia_id,
                               const std::string& username) {
  std::string account_id =
      account_tracker_service()->SeedAccountInfo(gaia_id, username);
  token_service()->UpdateCredentials(account_id, "test_refresh_token");
  OnExternalSigninCompleted(username);
}

void FakeSigninManager::OnSignoutDecisionReached(
    signin_metrics::ProfileSignout signout_source_metric,
    signin_metrics::SignoutDelete signout_delete_metric,
    RemoveAccountsOption remove_option,
    SigninClient::SignoutDecision signout_decision) {
  if (!IsAuthenticated()) {
    return;
  }

  // TODO(crbug.com/887756): Consider moving this higher up, or document why
  // the above blocks are exempt from the |signout_decision| early return.
  if (signout_decision == SigninClient::SignoutDecision::DISALLOW_SIGNOUT)
    return;

  AccountInfo account_info = GetAuthenticatedAccountInfo();
  const std::string account_id = GetAuthenticatedAccountId();
  const std::string username = account_info.email;
  authenticated_account_id_.clear();
  switch (remove_option) {
    case RemoveAccountsOption::kRemoveAllAccounts:
      if (token_service())
        token_service()->RevokeAllCredentials();
      break;
    case RemoveAccountsOption::kRemoveAuthenticatedAccountIfInError:
      if (token_service() && token_service()->RefreshTokenHasError(account_id))
        token_service()->RevokeCredentials(account_id);
      break;
    case RemoveAccountsOption::kKeepAllAccounts:
      // Do nothing.
      break;
  }
  ClearAuthenticatedAccountId();
  client_->GetPrefs()->ClearPref(prefs::kGoogleServicesHostedDomain);
  client_->GetPrefs()->ClearPref(prefs::kGoogleServicesAccountId);
  client_->GetPrefs()->ClearPref(prefs::kGoogleServicesUserAccountId);
  client_->GetPrefs()->ClearPref(prefs::kSignedInTime);

  FireGoogleSignedOut(account_info);
}

#endif  // !defined (OS_CHROMEOS)
