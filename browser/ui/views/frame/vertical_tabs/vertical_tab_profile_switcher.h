/* Copyright (c) 2026 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_VIEWS_FRAME_VERTICAL_TABS_VERTICAL_TAB_PROFILE_SWITCHER_H_
#define BRAVE_BROWSER_UI_VIEWS_FRAME_VERTICAL_TABS_VERTICAL_TAB_PROFILE_SWITCHER_H_

#include <string>

#include "base/files/file_path.h"
#include "base/memory/raw_ptr.h"
#include "base/scoped_observation.h"
#include "chrome/browser/profiles/profile_attributes_storage.h"
#include "ui/base/metadata/metadata_header_macros.h"
#include "ui/views/view.h"

class BrowserWindowInterface;

// "Come Dia": a row of profile avatars pinned to the bottom of the vertical
// tab strip. One click switches to that profile's window (opening one if
// needed). While the strip is collapsed only the current profile is shown;
// the full row appears when the strip floats on hover or is expanded.
class VerticalTabProfileSwitcher : public views::View,
                                   public ProfileAttributesStorage::Observer {
  METADATA_HEADER(VerticalTabProfileSwitcher, views::View)
 public:
  static constexpr int kAvatarSize = 24;
  static constexpr int kButtonSize = 32;
  static constexpr int kSpacing = 4;

  explicit VerticalTabProfileSwitcher(BrowserWindowInterface* browser);
  VerticalTabProfileSwitcher(const VerticalTabProfileSwitcher&) = delete;
  VerticalTabProfileSwitcher& operator=(const VerticalTabProfileSwitcher&) =
      delete;
  ~VerticalTabProfileSwitcher() override;

  // Shows only the current profile when |collapsed| is true.
  void SetCollapsed(bool collapsed);

  // views::View:
  gfx::Size CalculatePreferredSize(
      const views::SizeBounds& available_size) const override;

  // ProfileAttributesStorage::Observer:
  void OnProfileAdded(const base::FilePath& profile_path) override;
  void OnProfileWasRemoved(const base::FilePath& profile_path,
                           const std::u16string& profile_name) override;
  void OnProfileNameChanged(const base::FilePath& profile_path,
                            const std::u16string& old_profile_name) override;
  void OnProfileAvatarChanged(const base::FilePath& profile_path) override;
  void OnProfileHighResAvatarLoaded(
      const base::FilePath& profile_path) override;
  void OnProfileIsOmittedChanged(const base::FilePath& profile_path) override;

 private:
  void Rebuild();
  void UpdateVisibilityForState();
  void OnAvatarPressed(const base::FilePath& profile_path);

  raw_ptr<BrowserWindowInterface> browser_ = nullptr;
  base::FilePath current_profile_path_;
  raw_ptr<views::View> current_profile_button_ = nullptr;
  bool collapsed_ = false;

  base::ScopedObservation<ProfileAttributesStorage,
                          ProfileAttributesStorage::Observer>
      storage_observation_{this};
};

#endif  // BRAVE_BROWSER_UI_VIEWS_FRAME_VERTICAL_TABS_VERTICAL_TAB_PROFILE_SWITCHER_H_
