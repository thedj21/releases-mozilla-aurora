/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "MacIOSurfaceImage.h"
#include "mozilla/layers/TextureClientOGL.h"

using namespace mozilla::layers;

TextureClient*
MacIOSurfaceImage::GetTextureClient()
{
  if (!mTextureClient) {
    RefPtr<MacIOSurfaceTextureClientOGL> buffer =
      new MacIOSurfaceTextureClientOGL(TEXTURE_FLAGS_DEFAULT);
    buffer->InitWith(mSurface);
    mTextureClient = buffer;
  }
  return mTextureClient;
}
