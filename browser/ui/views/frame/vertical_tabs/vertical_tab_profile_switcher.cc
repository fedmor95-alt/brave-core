/* Copyright (c) 2026 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/views/frame/vertical_tabs/vertical_tab_profile_switcher.h"

#include <memory>
#include <utility>

#include "base/functional/bind.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_attributes_entry.h"
#include "chrome/browser/profiles/profile_avatar_icon_util.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/profiles/profile_window.h"
#include "chrome/browser/ui/browser_window/public/browser_window_interface.h"
#include "chrome/browser/ui/profiles/profile_picker.h"
#include "ui/base/metadata/metadata_impl_macros.h"
#include "ui/base/models/image_model.h"
#include "ui/color/color_id.h"
#include "ui/compositor/layer.h"
#include "ui/views/accessibility/view_accessibility.h"
#include "ui/views/animation/ink_drop.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/highlight_path_generator.h"
#include "ui/views/layout/box_layout.h"

namespace {

ProfileAttributesStorage* GetStorage() {
  if (!g_browser_process || !g_browser_process->profile_manager()) {
    return nullptr;
  }
  return &g_browser_process->profile_manager()->GetProfileAttributesStorage();
}

}  // namespace

VerticalTabProfileSwitcher::VerticalTabProfileSwitcher(
    BrowserWindowInterface* browser)
    : browser_(browser) {
  current_profile_path_ =
      browser_->GetProfile()->GetOriginalProfile()->GetPath();

  auto* layout = SetLayoutManager(std::make_unique<views::BoxLayout>(
      views::BoxLayout::Orientation::kHorizontal, gfx::Insets(), kSpacing));
  layout->set_main_axis_alignment(views::BoxLayout::MainAxisAlignment::kStart);
  layout->set_cross_axis_alignment(
      views::BoxLayout::CrossAxisAlignment::kCenter);

  // Avatars that don't fit while the strip animates are clipped.
  SetPaintToLayer();
  layer()->SetFillsBoundsOpaquely(false);
  layer()->SetMasksToBounds(true);

  if (auto* storage = GetStorage()) {
    storage_observation_.Observe(storage);
  }
  Rebuild();
}

VerticalTabProfileSwitcher::~VerticalTabProfileSwitcher() = default;

void VerticalTabProfileSwitcher::SetCollapsed(bool collapsed) {
  if (collapsed_ == collapsed) {
    return;
  }
  collapsed_ = collapsed;
  UpdateVisibilityForState();
}

gfx::Size VerticalTabProfileSwitcher::CalculatePreferredSize(
    const views::SizeBounds& available_size) const {
  // Fixed height, so the tab list above never jumps when profiles change.
  gfx::Size size = views::View::CalculatePreferredSize(available_size);
  size.set_height(kButtonSize);
  return size;
}

void VerticalTabProfileSwitcher::Rebuild() {
  current_profile_button_ = nullptr;
  RemoveAllChildViews();

  auto* storage = GetStorage();
  if (!storage) {
    return;
  }

  for (ProfileAttributesEntry* entry :
       storage->GetAllProfilesAttributesSortedForDisplay()) {
    if (entry->IsOmitted()) {
      continue;
    }
    const base::FilePath path = entry->GetPath();

    gfx::Image avatar = profiles::GetSizedAvatarIcon(
        entry->GetAvatarIcon(kAvatarSize), kAvatarSize, kAvatarSize,
        profiles::SHAPE_CIRCLE);

    auto button = std::make_unique<views::ImageButton>(
        base::BindRepeating(&VerticalTabProfileSwitcher::OnAvatarPressed,
                            base::Unretained(this), path));
    button->SetImageModel(views::Button::STATE_NORMAL,
                          ui::ImageModel::FromImage(avatar));
    button->SetImageHorizontalAlignment(views::ImageButton::ALIGN_CENTER);
    button->SetImageVerticalAlignment(views::ImageButton::ALIGN_MIDDLE);
    button->SetPreferredSize(gfx::Size(kButtonSize, kButtonSize));
    button->SetTooltipText(entry->GetName());
    button->GetViewAccessibility().SetName(entry->GetName());
    views::InkDrop::Get(button.get())
        ->SetMode(views::InkDropHost::InkDropMode::ON);
    views::InstallCircleHighlightPathGenerator(button.get());

    if (path == current_profile_path_) {
      // Ring around the active profile, like Dia's profile row.
      button->SetBorder(views::CreateRoundedRectBorder(
          /*thickness=*/2, /*corner_radius=*/kButtonSize / 2.0f,
          ui::kColorAccent));
      // Keep the current profile first so the collapsed strip shows it.
      current_profile_button_ = AddChildViewAt(std::move(button), 0);
    } else {
      AddChildView(std::move(button));
    }
  }

  UpdateVisibilityForState();
  PreferredSizeChanged();
}

void VerticalTabProfileSwitcher::UpdateVisibilityForState() {
  for (views::View* child : children()) {
    child->SetVisible(child == current_profile_button_ || !collapsed_);
  }
  InvalidateLayout();
}

void VerticalTabProfileSwitcher::OnAvatarPressed(
    const base::FilePath& profile_path) {
  if (profile_path == current_profile_path_) {
    // Clicking the active profile opens the profile manager, so adding or
    // editing profiles stays one click away.
    ProfilePicker::Show(ProfilePicker::Params::FromEntryPoint(
        ProfilePicker::EntryPoint::kProfileMenuManageProfiles));
    return;
  }
  // Focuses the profile's existing window, or opens a new one.
  profiles::SwitchToProfile(profile_path, /*always_create=*/false);
}

void VerticalTabProfileSwitcher::OnProfileAdded(
    const base::FilePath& profile_path) {
  Rebuild();
}

void VerticalTabProfileSwitcher::OnProfileWasRemoved(
    const base::FilePath& profile_path,
    const std::u16string& profile_name) {
  Rebuild();
}

void VerticalTabProfileSwitcher::OnProfileNameChanged(
    const base::FilePath& profile_path,
    const std::u16string& old_profile_name) {
  Rebuild();
}

void VerticalTabProfileSwitcher::OnProfileAvatarChanged(
    const base::FilePath& profile_path) {
  Rebuild();
}

void VerticalTabProfileSwitcher::OnProfileHighResAvatarLoaded(
    const base::FilePath& profile_path) {
  Rebuild();
}

void VerticalTabProfileSwitcher::OnProfileIsOmittedChanged(
    const base::FilePath& profile_path) {
  Rebuild();
}

BEGIN_METADATA(VerticalTabProfileSwitcher)
END_METADATA
