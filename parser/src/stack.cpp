/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */


 #include <stack.h>

void Stack::wireAdjacency()
{
    for (size_t i = 0; i < mLayers.size(); ++i) {
        LayerContext* lower = (i > 0) ? &mLayers[i - 1]->context : nullptr;
        LayerContext* upper = (i + 1 < mLayers.size())
                                ? &mLayers[i + 1]->context : nullptr;
        mLayers[i]->context.setLower(lower);
        mLayers[i]->context.setUpper(upper);
    }
}
