//  This file is licenced under the GNU Affero General Public License and the Resonate Supplemental Terms. (See file LICENSE at <https://github.com/98ahni/UnityWindowing?tab=GPL-3.0-1-ov-file>)
//  <Copyright (C) 2026 98ahni> Original file author

#pragma once

struct WindowCreationProperties
{
	bool alignas(1) Resizable;
	bool alignas(1) Decorated;
	bool alignas(1) Focused;
	bool alignas(1) AlwaysOnTop;
	bool alignas(1) Maximized;
	int alignas(4) PositionX;
	int alignas(4) PositionY;
};

// The alignas is to have predictable interoperability and corresponds to the byte offsets in C#
