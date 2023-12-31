/*
 * Copyright 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <android-base/thread_annotations.h>
#include <gui/DisplayEventReceiver.h>
#include <scheduler/Fps.h>
#include <sys/types.h>
#include <map>
#include <optional>

#include "Utils/Dumper.h"

namespace android::scheduler {

class FrameRateOverrideMappings {
    using FrameRateOverride = DisplayEventReceiver::Event::FrameRateOverride;
    using UidToFrameRateOverride = std::map<uid_t, Fps>;

public:
    std::optional<Fps> getFrameRateOverrideForUid(uid_t uid,
                                                  bool supportsFrameRateOverrideByContent) const
            EXCLUDES(mFrameRateOverridesLock);
    std::vector<FrameRateOverride> getAllFrameRateOverrides(bool supportsFrameRateOverrideByContent)
            EXCLUDES(mFrameRateOverridesLock);
    bool updateFrameRateOverridesByContent(const UidToFrameRateOverride& frameRateOverrides)
            EXCLUDES(mFrameRateOverridesLock);
    void setGameModeRefreshRateForUid(FrameRateOverride frameRateOverride)
            EXCLUDES(mFrameRateOverridesLock);
    void setPreferredRefreshRateForUid(FrameRateOverride frameRateOverride)
            EXCLUDES(mFrameRateOverridesLock);

    void dump(utils::Dumper&) const;

private:
    size_t maxOverridesCount() const REQUIRES(mFrameRateOverridesLock) {
        return std::max({mFrameRateOverridesByContent.size(),
                         mFrameRateOverridesFromGameManager.size(),
                         mFrameRateOverridesFromBackdoor.size()});
    }

    void dump(utils::Dumper&, std::string_view name, const UidToFrameRateOverride&) const;

    // The frame rate override lists need their own mutex as they are being read
    // by SurfaceFlinger, Scheduler and EventThread (as a callback) to prevent deadlocks
    mutable std::mutex mFrameRateOverridesLock;

    // mappings between a UID and a preferred refresh rate that this app would
    // run at.
    UidToFrameRateOverride mFrameRateOverridesByContent GUARDED_BY(mFrameRateOverridesLock);
    UidToFrameRateOverride mFrameRateOverridesFromBackdoor GUARDED_BY(mFrameRateOverridesLock);
    UidToFrameRateOverride mFrameRateOverridesFromGameManager GUARDED_BY(mFrameRateOverridesLock);
};

} // namespace android::scheduler
