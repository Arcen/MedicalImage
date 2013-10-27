////////////////////////////////////////////////////////////////////////////////
// Template Library

// ANSI関数を利用したテンプレートライブラリ
// コンテナ・文字列など基本構造のみをサポート
// 注意：assertマクロを外部で定義する必要がある．

#ifndef __INCLUDE_TL_HEADER__
#define __INCLUDE_TL_HEADER__

#pragma warning( push )
#pragma warning( disable : 4244 4284 4700 )

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#include "macro.h"
#include "point.h"
#include "rect.h"
#include "retainer.h"
#include "container.h"
#include "smart.h"
#include "string.h"
#include "map.h"
//#include "region.h"
#include "check.h"

#pragma warning( pop )

#endif
