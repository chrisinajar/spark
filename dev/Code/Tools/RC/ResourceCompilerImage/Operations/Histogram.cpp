/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

#include "stdafx.h"
#include <assert.h>                         // assert()

#include "../ImageCompiler.h"               // CImageCompiler
#include "../ImageObject.h"                 // ImageToProcess

#include "IRCLog.h"                         // IRCLog
#include "Histogram.h"
#include "MathHelpers.h"                    // FastRoundFloatTowardZero()
#include "../Converters/ColorSpaces/RGBL.h" // Luminance

///////////////////////////////////////////////////////////////////////////////////

// Compute histogram for BGR8, BGRA8, BGRX8, RGBA32F
bool ImageObject::ComputeLuminanceHistogramForAnyRGB(Histogram<256>& histogram) const
{
    ESampleType srcSampleType;
    int srcChannelCount;
    bool srcHasAlpha;

    if (!CPixelFormats::GetPixelFormatInfo(GetPixelFormat(), &srcSampleType, &srcChannelCount, &srcHasAlpha))
    {
        assert(0);
        return false;
    }

    static const uint32 dwMip = 0;

    char* pSrcMem;
    uint32 dwSrcPitch;
    GetImagePointer(dwMip, pSrcMem, dwSrcPitch);

    if (pSrcMem == 0)
    {
        assert(0);
        return false;
    }

    const uint32 dwLocalWidth = GetWidth(dwMip);
    const uint32 dwLocalHeight = GetHeight(dwMip);

    static const size_t binCount = 256;
    Histogram<binCount>::Bins bins;
    Histogram<binCount>::clearBins(bins);

    if (srcSampleType == eSampleType_Uint8)
    {
        for (uint32 dwY = 0; dwY < dwLocalHeight; ++dwY)
        {
            const unsigned char* pSrc = (const unsigned char*) &pSrcMem[dwSrcPitch * dwY];
            for (uint32 dwX = 0; dwX < dwLocalWidth; ++dwX)
            {
                const unsigned char luminance = RGBL::GetLuminance(pSrc[2], pSrc[1], pSrc[0]);  // [0] b, [1] g, [2] r
                ++bins[luminance];
                pSrc += srcChannelCount;
            }
        }
    }
    else if (srcSampleType == eSampleType_Uint16)
    {
        for (uint32 dwY = 0; dwY < dwLocalHeight; ++dwY)
        {
            const unsigned short* pSrc = (const unsigned short*) &pSrcMem[dwSrcPitch * dwY];
            for (uint32 dwX = 0; dwX < dwLocalWidth; ++dwX)
            {
                const unsigned short luminance = RGBL::GetLuminance(pSrc[2], pSrc[1], pSrc[0]);  // [0] b, [1] g, [2] r
                ++bins[luminance / 0x101];
                pSrc += srcChannelCount;
            }
        }
    }
    else if (srcSampleType == eSampleType_Half)
    {
        for (uint32 dwY = 0; dwY < dwLocalHeight; ++dwY)
        {
            const SHalf* pSrc = (const SHalf*) &pSrcMem[dwSrcPitch * dwY];
            for (uint32 dwX = 0; dwX < dwLocalWidth; ++dwX)
            {
                const float luminance = Util::getClamped(RGBL::GetLuminance<float>(pSrc[0], pSrc[1], pSrc[2]), 0.0f, 1.0f);  // [0] r, [1] g, [2] b
                const float f = luminance * binCount;
                if (f <= 0)
                {
                    ++bins[0];
                }
                else
                {
                    const int bin = MathHelpers::FastRoundFloatTowardZero(f);
                    ++bins[(bin < binCount) ? bin : binCount - 1];
                }
                pSrc += srcChannelCount;
            }
        }
    }
    else if (srcSampleType == eSampleType_Float)
    {
        for (uint32 dwY = 0; dwY < dwLocalHeight; ++dwY)
        {
            const float* pSrc = (const float*) &pSrcMem[dwSrcPitch * dwY];
            for (uint32 dwX = 0; dwX < dwLocalWidth; ++dwX)
            {
                const float luminance = Util::getClamped(RGBL::GetLuminance(pSrc[0], pSrc[1], pSrc[2]), 0.0f, 1.0f);  // [0] r, [1] g, [2] b
                const float f = luminance * binCount;
                if (f <= 0)
                {
                    ++bins[0];
                }
                else
                {
                    const int bin = MathHelpers::FastRoundFloatTowardZero(f);
                    ++bins[(bin < binCount) ? bin : binCount - 1];
                }
                pSrc += srcChannelCount;
            }
        }
    }
    else
    {
        return false;
    }

    histogram.set(bins);

    return true;
}

// Compute vector length histogram for BGR8, BGRA8, BGRX8, RGBA32F
bool ImageObject::ComputeUnitSphereDeviationHistogramForAnyRGB(Histogram<256>& histogram) const
{
    ESampleType srcSampleType;
    int srcChannelCount;
    bool srcHasAlpha;

    if (!CPixelFormats::GetPixelFormatInfo(GetPixelFormat(), &srcSampleType, &srcChannelCount, &srcHasAlpha))
    {
        assert(0);
        return false;
    }

    static const uint32 dwMip = 0;

    char* pSrcMem;
    uint32 dwSrcPitch;
    GetImagePointer(dwMip, pSrcMem, dwSrcPitch);

    if (pSrcMem == 0)
    {
        assert(0);
        return false;
    }

    const uint32 dwLocalWidth = GetWidth(dwMip);
    const uint32 dwLocalHeight = GetHeight(dwMip);

    static const size_t binCount = 256;
    Histogram<binCount>::Bins bins;
    Histogram<binCount>::clearBins(bins);

    if (srcSampleType == eSampleType_Uint8)
    {
        for (uint32 dwY = 0; dwY < dwLocalHeight; ++dwY)
        {
            const unsigned char* pSrc = (const unsigned char*) &pSrcMem[dwSrcPitch * dwY];
            for (uint32 dwX = 0; dwX < dwLocalWidth; ++dwX)
            {
                const float length = sqrtf(Util::square(pSrc[0] - 127.5f) + Util::square(pSrc[1] - 127.5f) + Util::square(pSrc[2] - 127.5f)) / 127.5f;
                const float f = length * 0.5f * binCount;
                if (f <= 0)
                {
                    ++bins[0];
                }
                else
                {
                    const int bin = MathHelpers::FastRoundFloatTowardZero(f);
                    ++bins[(bin < binCount) ? bin : binCount - 1];
                }
                pSrc += srcChannelCount;
            }
        }
    }
    else if (srcSampleType == eSampleType_Uint16)
    {
        for (uint32 dwY = 0; dwY < dwLocalHeight; ++dwY)
        {
            const unsigned short* pSrc = (const unsigned short*) &pSrcMem[dwSrcPitch * dwY];
            for (uint32 dwX = 0; dwX < dwLocalWidth; ++dwX)
            {
                const float length = sqrtf(Util::square(pSrc[0] - 32767.5f) + Util::square(pSrc[1] - 32767.5f) + Util::square(pSrc[2] - 32767.5f)) / 32767.5f;
                const float f = length * 0.5f * binCount;
                if (f <= 0)
                {
                    ++bins[0];
                }
                else
                {
                    const int bin = MathHelpers::FastRoundFloatTowardZero(f);
                    ++bins[(bin < binCount) ? bin : binCount - 1];
                }
                pSrc += srcChannelCount;
            }
        }
    }
    else if (srcSampleType == eSampleType_Half)
    {
        for (uint32 dwY = 0; dwY < dwLocalHeight; ++dwY)
        {
            const SHalf* pSrc = (const SHalf*) &pSrcMem[dwSrcPitch * dwY];
            for (uint32 dwX = 0; dwX < dwLocalWidth; ++dwX)
            {
                const float length = sqrtf(Util::square(pSrc[0] - 0.5f) + Util::square(pSrc[1] - 0.5f) + Util::square(pSrc[2] - 0.5f)) / 0.5f;
                const float f = length * 0.5f * binCount;
                if (f <= 0)
                {
                    ++bins[0];
                }
                else
                {
                    const int bin = MathHelpers::FastRoundFloatTowardZero(f);
                    ++bins[(bin < binCount) ? bin : binCount - 1];
                }
                pSrc += srcChannelCount;
            }
        }
    }
    else if (srcSampleType == eSampleType_Float)
    {
        for (uint32 dwY = 0; dwY < dwLocalHeight; ++dwY)
        {
            const float* pSrc = (const float*) &pSrcMem[dwSrcPitch * dwY];
            for (uint32 dwX = 0; dwX < dwLocalWidth; ++dwX)
            {
                const float length = sqrtf(Util::square(pSrc[0] - 0.5f) + Util::square(pSrc[1] - 0.5f) + Util::square(pSrc[2] - 0.5f)) / 0.5f;
                const float f = length * 0.5f * binCount;
                if (f <= 0)
                {
                    ++bins[0];
                }
                else
                {
                    const int bin = MathHelpers::FastRoundFloatTowardZero(f);
                    ++bins[(bin < binCount) ? bin : binCount - 1];
                }
                pSrc += srcChannelCount;
            }
        }
    }
    else
    {
        return false;
    }

    histogram.set(bins);

    return true;
}