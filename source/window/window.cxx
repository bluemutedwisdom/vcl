/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_features.h>

#include <i18nlangtag/mslangid.hxx>

#include "tools/time.hxx"
#include "tools/debug.hxx"
#include "tools/rc.h"

#include "unotools/fontcfg.hxx"
#include "unotools/confignode.hxx"

#include "vcl/layout.hxx"
#include "vcl/salgtype.hxx"
#include "vcl/event.hxx"
#include "vcl/fixed.hxx"
#include "vcl/help.hxx"
#include "vcl/cursor.hxx"
#include "vcl/svapp.hxx"
#include "vcl/window.hxx"
#include "vcl/syswin.hxx"
#include "vcl/syschild.hxx"
#include "vcl/dockwin.hxx"
#include "vcl/menu.hxx"
#include "vcl/wrkwin.hxx"
#include "vcl/wall.hxx"
#include "vcl/gradient.hxx"
#include "vcl/button.hxx"
#include "vcl/taskpanelist.hxx"
#include "vcl/dialog.hxx"
#include "vcl/unowrap.hxx"
#include "vcl/gdimtf.hxx"
#include "vcl/pdfextoutdevdata.hxx"
#include "vcl/popupmenuwindow.hxx"
#include "vcl/lazydelete.hxx"
#include "vcl/virdev.hxx"
#include "vcl/settings.hxx"

// declare system types in sysdata.hxx
#include "svsys.h"
#include "vcl/sysdata.hxx"

#include "salframe.hxx"
#include "salobj.hxx"
#include "salinst.hxx"
#include "salgdi.hxx"
#include "svdata.hxx"
#include "dbggui.hxx"
#include "outfont.hxx"
#include "window.h"
#include "toolbox.h"
#include "outdev.h"
#include "PhysicalFontCollection.hxx"
#include "brdwin.hxx"
#include "helpwin.hxx"
#include "sallayout.hxx"
#include "dndlcon.hxx"
#include "dndevdis.hxx"

#include "com/sun/star/accessibility/XAccessible.hpp"
#include "com/sun/star/accessibility/AccessibleRole.hpp"
#include "com/sun/star/awt/XWindowPeer.hpp"
#include "com/sun/star/awt/XTopWindow.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/awt/XDisplayConnection.hpp"
#include "com/sun/star/datatransfer/dnd/XDragSource.hpp"
#include "com/sun/star/datatransfer/dnd/XDropTarget.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboard.hpp"
#include "com/sun/star/datatransfer/clipboard/SystemClipboard.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XServiceName.hpp"
#include "com/sun/star/rendering/CanvasFactory.hpp"
#include "com/sun/star/rendering/XCanvas.hpp"
#include "com/sun/star/rendering/XSpriteCanvas.hpp"
#include "comphelper/processfactory.hxx"

#include <sal/macros.h>
#include <rtl/strbuf.hxx>

#include <set>
#include <typeinfo>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;
using namespace ::com::sun::star;
using namespace com::sun;

using ::com::sun::star::awt::XTopWindow;

#define IMPL_PAINT_PAINT            ((sal_uInt16)0x0001)
#define IMPL_PAINT_PAINTALL         ((sal_uInt16)0x0002)
#define IMPL_PAINT_PAINTALLCHILDREN   ((sal_uInt16)0x0004)
#define IMPL_PAINT_PAINTCHILDREN      ((sal_uInt16)0x0008)
#define IMPL_PAINT_ERASE            ((sal_uInt16)0x0010)
#define IMPL_PAINT_CHECKRTL         ((sal_uInt16)0x0020)

WindowImpl::WindowImpl( WindowType nType )
{
    maZoom              = Fraction( 1, 1 );
    maWinRegion         = Region(true);
    maWinClipRegion     = Region(true);
    mpWinData           = NULL;         // Extra Window Data, that we dont need for all windows
    mpOverlapData       = NULL;         // Overlap Data
    mpFrameData         = NULL;         // Frame Data
    mpFrame             = NULL;         // Pointer to frame window
    mpSysObj            = NULL;
    mpFrameWindow       = NULL;         // window to top level parent (same as frame window)
    mpOverlapWindow     = NULL;         // first overlap parent
    mpBorderWindow      = NULL;         // Border-Window
    mpClientWindow      = NULL;         // Client-Window of a FrameWindow
    mpParent            = NULL;         // parent (inkl. BorderWindow)
    mpRealParent        = NULL;         // real parent (exkl. BorderWindow)
    mpFirstChild        = NULL;         // first child window
    mpLastChild         = NULL;         // last child window
    mpFirstOverlap      = NULL;         // first overlap window (only set in overlap windows)
    mpLastOverlap       = NULL;         // last overlap window (only set in overlap windows)
    mpPrev              = NULL;         // prev window
    mpNext              = NULL;         // next window
    mpNextOverlap       = NULL;         // next overlap window of frame
    mpLastFocusWindow   = NULL;         // window for focus restore
    mpDlgCtrlDownWindow = NULL;         // window for dialog control
    mpFirstDel          = NULL;         // Dtor notification list
    mpUserData          = NULL;         // user data
    mpCursor            = NULL;         // cursor
    mpControlFont       = NULL;         // font propertie
    mpVCLXWindow        = NULL;
    mpAccessibleInfos   = NULL;
    maControlForeground = Color( COL_TRANSPARENT );     // no foreground set
    maControlBackground = Color( COL_TRANSPARENT );     // no background set
    mnLeftBorder        = 0;            // left border
    mnTopBorder         = 0;            // top border
    mnRightBorder       = 0;            // right border
    mnBottomBorder      = 0;            // bottom border
    mnWidthRequest      = -1;           // width request
    mnHeightRequest     = -1;           // height request
    mnX                 = 0;            // X-Position to Parent
    mnY                 = 0;            // Y-Position to Parent
    mnAbsScreenX        = 0;            // absolute X-position on screen, used for RTL window positioning
    mpChildClipRegion   = NULL;         // Child-Clip-Region when ClipChildren
    mpPaintRegion       = NULL;         // Paint-ClipRegion
    mnStyle             = 0;            // style (init in ImplInitWindow)
    mnPrevStyle         = 0;            // prevstyle (set in SetStyle)
    mnExtendedStyle     = 0;            // extended style (init in ImplInitWindow)
    mnPrevExtendedStyle = 0;            // prevstyle (set in SetExtendedStyle)
    mnType              = nType;        // type
    mnGetFocusFlags     = 0;            // Flags fuer GetFocus()-Aufruf
    mnWaitCount         = 0;            // Wait-Count (>1 == Warte-MousePointer)
    mnPaintFlags        = 0;            // Flags for ImplCallPaint
    mnParentClipMode    = 0;            // Flags for Parent-ClipChildren-Mode
    mnActivateMode      = 0;            // Will be converted in System/Overlap-Windows
    mnDlgCtrlFlags      = 0;            // DialogControl-Flags
    mnLockCount         = 0;            // LockCount
    meAlwaysInputMode   = AlwaysInputNone; // neither AlwaysEnableInput nor AlwaysDisableInput called
    meHalign            = VCL_ALIGN_FILL;
    meValign            = VCL_ALIGN_FILL;
    mePackType          = VCL_PACK_START;
    mnPadding           = 0;
    mnGridHeight        = 1;
    mnGridLeftAttach    = -1;
    mnGridTopAttach     = -1;
    mnGridWidth         = 1;
    mnBorderWidth       = 0;
    mnMarginLeft        = 0;
    mnMarginRight       = 0;
    mnMarginTop         = 0;
    mnMarginBottom      = 0;
    mbFrame             = false;        // true: Window is a frame window
    mbBorderWin         = false;        // true: Window is a border window
    mbOverlapWin        = false;        // true: Window is a overlap window
    mbSysWin            = false;        // true: SystemWindow is the base class
    mbDialog            = false;        // true: Dialog is the base class
    mbDockWin           = false;        // true: DockingWindow is the base class
    mbFloatWin          = false;        // true: FloatingWindow is the base class
    mbPushButton        = false;        // true: PushButton is the base class
    mbToolBox           = false;      // true: ToolBox is the base class
    mbMenuFloatingWindow= false;      // true: MenuFloatingWindow is the base class
    mbToolbarFloatingWindow= false;       // true: ImplPopupFloatWin is the base class, used for subtoolbars
    mbSplitter          = false;      // true: Splitter is the base class
    mbVisible           = false;        // true: Show( sal_True ) called
    mbOverlapVisible    = false;        // true: Hide called for visible window from ImplHideAllOverlapWindow()
    mbDisabled          = false;        // true: Enable( sal_False ) called
    mbInputDisabled     = false;        // true: EnableInput( sal_False ) called
    mbDropDisabled      = false;        // true: Drop is enabled
    mbNoUpdate          = false;        // true: SetUpdateMode( sal_False ) called
    mbNoParentUpdate    = false;        // true: SetParentUpdateMode( sal_False ) called
    mbActive            = false;        // true: Window Active
    mbParentActive      = false;        // true: OverlapActive from Parent
    mbReallyVisible     = false;        // true: this and all parents to an overlapped window are visible
    mbReallyShown       = false;        // true: this and all parents to an overlapped window are shown
    mbInInitShow        = false;        // true: we are in InitShow
    mbChildNotify       = false;        // true: ChildNotify
    mbChildPtrOverwrite = false;        // true: PointerStyle overwrites Child-Pointer
    mbNoPtrVisible      = false;        // true: ShowPointer( sal_False ) called
    mbMouseMove         = false;        // true: BaseMouseMove called
    mbPaintFrame        = false;        // true: Paint is visible, but not painted
    mbInPaint           = false;        // true: Inside PaintHdl
    mbMouseButtonDown   = false;        // true: BaseMouseButtonDown called
    mbMouseButtonUp     = false;        // true: BaseMouseButtonUp called
    mbKeyInput          = false;        // true: BaseKeyInput called
    mbKeyUp             = false;        // true: BaseKeyUp called
    mbCommand           = false;        // true: BaseCommand called
    mbDefPos            = true;         // true: Position is not Set
    mbDefSize           = true;         // true: Size is not Set
    mbCallMove          = true;         // true: Move must be called by Show
    mbCallResize        = true;         // true: Resize must be called by Show
    mbWaitSystemResize  = true;         // true: Wait for System-Resize
    mbInitWinClipRegion = true;         // true: Calc Window Clip Region
    mbInitChildRegion   = false;        // true: InitChildClipRegion
    mbWinRegion         = false;        // true: Window Region
    mbClipChildren      = false;        // true: Child-window should be clipped
    mbClipSiblings      = false;        // true: Adjacent Child-window should be clipped
    mbChildTransparent  = false;        // true: Child-windows are allowed to switch to transparent (incl. Parent-CLIPCHILDREN)
    mbPaintTransparent  = false;        // true: Paints should be executed on the Parent
    mbMouseTransparent  = false;        // true: Window is transparent for Mouse
    mbDlgCtrlStart      = false;        // true: From here on own Dialog-Control
    mbFocusVisible      = false;        // true: Focus Visible
    mbUseNativeFocus    = false;
    mbNativeFocusVisible= false;        // true: native Focus Visible
    mbInShowFocus       = false;        // prevent recursion
    mbInHideFocus       = false;        // prevent recursion
    mbTrackVisible      = false;        // true: Tracking Visible
    mbControlForeground = false;        // true: Foreground-Property set
    mbControlBackground = false;        // true: Background-Property set
    mbAlwaysOnTop       = false;        // true: always visible for all others windows
    mbCompoundControl   = false;        // true: Composite Control => Listener...
    mbCompoundControlHasFocus = false;  // true: Composite Control has focus somewhere
    mbPaintDisabled     = false;        // true: Paint should not be executed
    mbAllResize         = false;        // true: Also sent ResizeEvents with 0,0
    mbInDtor            = false;        // true: We're still in Window-Dtor
    mbExtTextInput      = false;        // true: ExtTextInput-Mode is active
    mbInFocusHdl        = false;        // true: Within GetFocus-Handler
    mbCreatedWithToolkit = false;
    mbSuppressAccessibilityEvents = false; // true: do not send any accessibility events
    mbDrawSelectionBackground = false;    // true: draws transparent window background to indicate (toolbox) selection
    mbIsInTaskPaneList = false;           // true: window was added to the taskpanelist in the topmost system window
    mnNativeBackground  = 0;              // initialize later, depends on type
    mbCallHandlersDuringInputDisabled = false; // true: call event handlers even if input is disabled
    mbHelpTextDynamic = false;          // true: append help id in HELP_DEBUG case
    mbFakeFocusSet = false; // true: pretend as if the window has focus.
    mbHexpand = false;
    mbVexpand = false;
    mbExpand = false;
    mbFill = true;
    mbSecondary = false;
    mbNonHomogeneous = false;
}

WindowImpl::~WindowImpl()
{
    delete mpChildClipRegion;
    delete mpAccessibleInfos;
    delete mpControlFont;
}

Window::Window( WindowType nType )
{
    ImplInitWindowData( nType );
}

Window::Window( Window* pParent, WinBits nStyle )
{

    ImplInitWindowData( WINDOW_WINDOW );
    ImplInit( pParent, nStyle, NULL );
}

Window::Window( Window* pParent, const ResId& rResId )
    : mpWindowImpl(NULL)
{
    rResId.SetRT( RSC_WINDOW );
    WinBits nStyle = ImplInitRes( rResId );
    ImplInitWindowData( WINDOW_WINDOW );
    ImplInit( pParent, nStyle, NULL );
    ImplLoadRes( rResId );

    if ( !(nStyle & WB_HIDE) )
        Show();
}

bool Window::AcquireGraphics() const
{
    DBG_TESTSOLARMUTEX();

    if ( mpGraphics )
        return true;

    mbInitLineColor     = true;
    mbInitFillColor     = true;
    mbInitFont          = true;
    mbInitTextColor     = true;
    mbInitClipRegion    = true;

    ImplSVData* pSVData = ImplGetSVData();

    mpGraphics = mpWindowImpl->mpFrame->AcquireGraphics();
    // try harder if no wingraphics was available directly
    if ( !mpGraphics )
    {
        // find another output device in the same frame
        OutputDevice* pReleaseOutDev = pSVData->maGDIData.mpLastWinGraphics;
        while ( pReleaseOutDev )
        {
            if ( ((Window*)pReleaseOutDev)->mpWindowImpl->mpFrame == mpWindowImpl->mpFrame )
                break;
            pReleaseOutDev = pReleaseOutDev->mpPrevGraphics;
        }

        if ( pReleaseOutDev )
        {
            // steal the wingraphics from the other outdev
            mpGraphics = pReleaseOutDev->mpGraphics;
            pReleaseOutDev->ReleaseGraphics( false );
        }
        else
        {
            // if needed retry after releasing least recently used wingraphics
            while ( !mpGraphics )
            {
                if ( !pSVData->maGDIData.mpLastWinGraphics )
                    break;
                pSVData->maGDIData.mpLastWinGraphics->ReleaseGraphics();
                mpGraphics = mpWindowImpl->mpFrame->AcquireGraphics();
            }
        }
    }

    // update global LRU list of wingraphics
    if ( mpGraphics )
    {
        mpNextGraphics = pSVData->maGDIData.mpFirstWinGraphics;
        pSVData->maGDIData.mpFirstWinGraphics = const_cast<Window*>(this);
        if ( mpNextGraphics )
            mpNextGraphics->mpPrevGraphics = const_cast<Window*>(this);
        if ( !pSVData->maGDIData.mpLastWinGraphics )
            pSVData->maGDIData.mpLastWinGraphics = const_cast<Window*>(this);
    }

    if ( mpGraphics )
    {
        mpGraphics->SetXORMode( (ROP_INVERT == meRasterOp) || (ROP_XOR == meRasterOp), ROP_INVERT == meRasterOp );
        mpGraphics->setAntiAliasB2DDraw(mnAntialiasing & ANTIALIASING_ENABLE_B2DDRAW);
    }

    return mpGraphics ? true : false;
}

void Window::ReleaseGraphics( bool bRelease )
{
    DBG_TESTSOLARMUTEX();

    if ( !mpGraphics )
        return;

    // release the fonts of the physically released graphics device
    if( bRelease )
        ImplReleaseFonts();

    ImplSVData* pSVData = ImplGetSVData();

    Window* pWindow = (Window*)this;

    if ( bRelease )
        pWindow->mpWindowImpl->mpFrame->ReleaseGraphics( mpGraphics );
    // remove from global LRU list of window graphics
    if ( mpPrevGraphics )
        mpPrevGraphics->mpNextGraphics = mpNextGraphics;
    else
        pSVData->maGDIData.mpFirstWinGraphics = mpNextGraphics;
    if ( mpNextGraphics )
        mpNextGraphics->mpPrevGraphics = mpPrevGraphics;
    else
        pSVData->maGDIData.mpLastWinGraphics = mpPrevGraphics;

    mpGraphics      = NULL;
    mpPrevGraphics  = NULL;
    mpNextGraphics  = NULL;
}

void Window::CopyDeviceArea( SalTwoRect& aPosAry, sal_uInt32 nFlags )
{
    if (aPosAry.mnSrcWidth == 0 || aPosAry.mnSrcHeight == 0 || aPosAry.mnDestWidth == 0 || aPosAry.mnDestHeight == 0)
        return;

    if (nFlags & COPYAREA_WINDOWINVALIDATE)
    {
        const Rectangle aSrcRect(Point(aPosAry.mnSrcX, aPosAry.mnSrcY),
                Size(aPosAry.mnSrcWidth, aPosAry.mnSrcHeight));

        ImplMoveAllInvalidateRegions(aSrcRect,
                aPosAry.mnDestX-aPosAry.mnSrcX,
                aPosAry.mnDestY-aPosAry.mnSrcY,
                false);

        mpGraphics->CopyArea(aPosAry.mnDestX, aPosAry.mnDestY,
                aPosAry.mnSrcX, aPosAry.mnSrcY,
                aPosAry.mnSrcWidth, aPosAry.mnSrcHeight,
                SAL_COPYAREA_WINDOWINVALIDATE, this);

        return;
    }

    OutputDevice::CopyDeviceArea(aPosAry, nFlags);
}

void Window::ImplInitAppFontData( Window* pWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    long nTextHeight = pWindow->GetTextHeight();
    long nTextWidth = pWindow->approximate_char_width() * 8;
    long nSymHeight = nTextHeight*4;
    // Make the basis wider if the font is too narrow
    // such that the dialog looks symmetrical and does not become too narrow.
    // Add some extra space when the dialog has the same width,
    // as a little more space is better.
    if ( nSymHeight > nTextWidth )
        nTextWidth = nSymHeight;
    else if ( nSymHeight+5 > nTextWidth )
        nTextWidth = nSymHeight+5;
    pSVData->maGDIData.mnAppFontX = nTextWidth * 10 / 8;
    pSVData->maGDIData.mnAppFontY = nTextHeight * 10;

    // FIXME: this is currently only on OS X, check with other
    // platforms
    if( pSVData->maNWFData.mbNoFocusRects )
    {
        // try to find out whether there is a large correction
        // of control sizes, if yes, make app font scalings larger
        // so dialog positioning is not completely off
        ImplControlValue aControlValue;
        Rectangle aCtrlRegion( Point(), Size( nTextWidth < 10 ? 10 : nTextWidth, nTextHeight < 10 ? 10 : nTextHeight ) );
        Rectangle aBoundingRgn( aCtrlRegion );
        Rectangle aContentRgn( aCtrlRegion );
        if( pWindow->GetNativeControlRegion( CTRL_EDITBOX, PART_ENTIRE_CONTROL, aCtrlRegion,
                                             CTRL_STATE_ENABLED, aControlValue, OUString(),
                                             aBoundingRgn, aContentRgn ) )
        {
            // comment: the magical +6 is for the extra border in bordered
            // (which is the standard) edit fields
            if( aContentRgn.GetHeight() - nTextHeight > (nTextHeight+4)/4 )
                pSVData->maGDIData.mnAppFontY = (aContentRgn.GetHeight()-4) * 10;
        }
    }

    pSVData->maGDIData.mnRealAppFontX = pSVData->maGDIData.mnAppFontX;
    if ( pSVData->maAppData.mnDialogScaleX )
        pSVData->maGDIData.mnAppFontX += (pSVData->maGDIData.mnAppFontX*pSVData->maAppData.mnDialogScaleX)/100;
}

bool Window::ImplCheckUIFont( const Font& rFont )
{
    if( ImplGetSVData()->maGDIData.mbNativeFontConfig )
        return true;

    // create a text string using the localized text of important buttons
    OUString aTestText;
    static const StandardButtonType aTestButtons[] =
    {
        BUTTON_OK, BUTTON_CANCEL, BUTTON_CLOSE, BUTTON_ABORT,
        BUTTON_YES, BUTTON_NO, BUTTON_MORE, BUTTON_IGNORE,
        BUTTON_RETRY, BUTTON_HELP
    };

    const int nTestButtonCount = SAL_N_ELEMENTS(aTestButtons);
    for( int n = 0; n < nTestButtonCount; ++n )
    {
        OUString aButtonStr = Button::GetStandardText( aTestButtons[n] );
        // #i115432# ignore mnemonic+accelerator part of each string
        // TODO: use a string filtering method when it becomes available
        const sal_Int32 nLen = aButtonStr.getLength();
        bool bInside = false;
        for( int i = 0; i < nLen; ++i ) {
            const sal_Unicode c = aButtonStr[ i ];
            if( (c == '('))
                bInside = true;
            if( (c == ')'))
                bInside = false;
            if( (c == '~')
            ||  (c == '(') || (c == ')')
            || ((c >= 'A') && (c <= 'Z') && bInside) )
                aButtonStr = aButtonStr.replaceAt( i, 1, " " );
        }
        // append sanitized button text to test string
        aTestText += aButtonStr;
    }

    const bool bUIFontOk = ( HasGlyphs( rFont, aTestText ) == -1 );
    return bUIFontOk;
}

void Window::ImplUpdateGlobalSettings( AllSettings& rSettings, bool bCallHdl )
{
    StyleSettings aTmpSt( rSettings.GetStyleSettings() );
    aTmpSt.SetHighContrastMode( false );
    rSettings.SetStyleSettings( aTmpSt );
    ImplGetFrame()->UpdateSettings( rSettings );
    // reset default border width for layouters
    ImplGetSVData()->maAppData.mnDefaultLayoutBorder = -1;

    // Verify availability of the configured UI font, otherwise choose "Andale Sans UI"

    // WTF, what makes Andale Sans UI a suitable cross-platform fallback font?

    OUString aUserInterfaceFont;
    bool bUseSystemFont = rSettings.GetStyleSettings().GetUseSystemUIFonts();

    // check whether system UI font can display a typical UI text
    if( bUseSystemFont )
        bUseSystemFont = ImplCheckUIFont( rSettings.GetStyleSettings().GetAppFont() );

    if ( !bUseSystemFont )
    {
        OutputDevice *pOutDev = GetOutDev();
        pOutDev->ImplInitFontList();
        OUString aConfigFont = utl::DefaultFontConfiguration::get().getUserInterfaceFont( rSettings.GetUILanguageTag() );
        sal_Int32 nIndex = 0;
        while( nIndex != -1 )
        {
            OUString aName( aConfigFont.getToken( 0, ';', nIndex ) );
            if ( !aName.isEmpty() && mpWindowImpl->mpFrameData->mpFontCollection->FindFontFamily( aName ) )
            {
                aUserInterfaceFont = aConfigFont;
                break;
            }
        }

        if ( aUserInterfaceFont.isEmpty() )
        {
            OUString aFallbackFont ("Andale Sans UI" );
            if ( mpWindowImpl->mpFrameData->mpFontCollection->FindFontFamily( aFallbackFont ) )
                aUserInterfaceFont = aFallbackFont;
        }
    }

    if ( !bUseSystemFont && !aUserInterfaceFont.isEmpty() )
    {
        StyleSettings aStyleSettings = rSettings.GetStyleSettings();
        Font aFont = aStyleSettings.GetAppFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetAppFont( aFont );
        aFont = aStyleSettings.GetHelpFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetHelpFont( aFont );
        aFont = aStyleSettings.GetTitleFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetTitleFont( aFont );
        aFont = aStyleSettings.GetFloatTitleFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetFloatTitleFont( aFont );
        aFont = aStyleSettings.GetMenuFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetMenuFont( aFont );
        aFont = aStyleSettings.GetToolFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetToolFont( aFont );
        aFont = aStyleSettings.GetLabelFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetLabelFont( aFont );
        aFont = aStyleSettings.GetInfoFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetInfoFont( aFont );
        aFont = aStyleSettings.GetRadioCheckFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetRadioCheckFont( aFont );
        aFont = aStyleSettings.GetPushButtonFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetPushButtonFont( aFont );
        aFont = aStyleSettings.GetFieldFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetFieldFont( aFont );
        aFont = aStyleSettings.GetIconFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetIconFont( aFont );
        aFont = aStyleSettings.GetGroupFont();
        aFont.SetName( aUserInterfaceFont );
        aStyleSettings.SetGroupFont( aFont );
        rSettings.SetStyleSettings( aStyleSettings );
    }

    StyleSettings aStyleSettings = rSettings.GetStyleSettings();
    // #97047: Force all fonts except Menu and Help to a fixed height
    // to avoid UI scaling due to large fonts
    // - but allow bigger fonts on bigger screens (i16682, i21238)
    //   dialogs were designed to fit 800x600 with an 8pt font, so scale accordingly
    int maxFontheight = 9; // #107886#: 9 is default for some asian systems, so always allow if requested
    if( GetDesktopRectPixel().getHeight() > 600 )
        maxFontheight = (int) ((( 8.0 * (double) GetDesktopRectPixel().getHeight()) / 600.0) + 1.5);

    Font aFont = aStyleSettings.GetMenuFont();
    int defFontheight = aFont.GetHeight();
    if( defFontheight > maxFontheight )
        defFontheight = maxFontheight;

    // if the UI is korean, chinese or another locale
    // where the system font size is kown to be often too small to
    // generate readable fonts enforce a minimum font size of 9 points
    bool bBrokenLangFontHeight = MsLangId::isCJK(Application::GetSettings().GetUILanguageTag().getLanguageType());
    if (bBrokenLangFontHeight)
        defFontheight = std::max(9, defFontheight);

    // i22098, toolfont will be scaled differently to avoid bloated rulers and status bars for big fonts
    int toolfontheight = defFontheight;
    if( toolfontheight > 9 )
        toolfontheight = (defFontheight+8) / 2;

    aFont = aStyleSettings.GetAppFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetAppFont( aFont );
    aFont = aStyleSettings.GetTitleFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetTitleFont( aFont );
    aFont = aStyleSettings.GetFloatTitleFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetFloatTitleFont( aFont );
    // keep menu and help font size from system unless in broken locale size
    if( bBrokenLangFontHeight )
    {
        aFont = aStyleSettings.GetMenuFont();
        if( aFont.GetHeight() < defFontheight )
        {
            aFont.SetHeight( defFontheight );
            aStyleSettings.SetMenuFont( aFont );
        }
        aFont = aStyleSettings.GetHelpFont();
        if( aFont.GetHeight() < defFontheight )
        {
            aFont.SetHeight( defFontheight );
            aStyleSettings.SetHelpFont( aFont );
        }
    }

    // use different height for toolfont
    aFont = aStyleSettings.GetToolFont();
    aFont.SetHeight( toolfontheight );
    aStyleSettings.SetToolFont( aFont );

    aFont = aStyleSettings.GetLabelFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetLabelFont( aFont );
    aFont = aStyleSettings.GetInfoFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetInfoFont( aFont );
    aFont = aStyleSettings.GetRadioCheckFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetRadioCheckFont( aFont );
    aFont = aStyleSettings.GetPushButtonFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetPushButtonFont( aFont );
    aFont = aStyleSettings.GetFieldFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetFieldFont( aFont );
    aFont = aStyleSettings.GetIconFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetIconFont( aFont );
    aFont = aStyleSettings.GetGroupFont();
    aFont.SetHeight( defFontheight );
    aStyleSettings.SetGroupFont( aFont );

    rSettings.SetStyleSettings( aStyleSettings );

    bool bForceHCMode = false;

    // auto detect HC mode; if the system already set it to "yes"
    // (see above) then accept that
    if( !rSettings.GetStyleSettings().GetHighContrastMode() )
    {
        bool bTmp = false, bAutoHCMode = true;
        utl::OConfigurationNode aNode = utl::OConfigurationTreeRoot::tryCreateWithComponentContext(
            comphelper::getProcessComponentContext(),
            OUString("org.openoffice.Office.Common/Accessibility") );    // note: case sensitive !
        if ( aNode.isValid() )
        {
            ::com::sun::star::uno::Any aValue = aNode.getNodeValue( OUString("AutoDetectSystemHC") );
            if( aValue >>= bTmp )
                bAutoHCMode = bTmp;
        }
        if( bAutoHCMode )
        {
            if( rSettings.GetStyleSettings().GetFaceColor().IsDark() ||
                rSettings.GetStyleSettings().GetWindowColor().IsDark() )
                bForceHCMode = true;
        }
    }

    static const char* pEnvHC = getenv( "SAL_FORCE_HC" );
    if( pEnvHC && *pEnvHC )
        bForceHCMode = true;

    if( bForceHCMode )
    {
        aStyleSettings = rSettings.GetStyleSettings();
        aStyleSettings.SetHighContrastMode( true );
        rSettings.SetStyleSettings( aStyleSettings );
    }

#if defined(DBG_UTIL)
    // If needed, set AppFont to bold, in order to check
    // if there is enough space available for texts on other systems
    if ( DbgIsBoldAppFont() )
    {
        aStyleSettings = rSettings.GetStyleSettings();
        aFont = aStyleSettings.GetAppFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetAppFont( aFont );
        aFont = aStyleSettings.GetGroupFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetGroupFont( aFont );
        aFont = aStyleSettings.GetLabelFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetLabelFont( aFont );
        aFont = aStyleSettings.GetRadioCheckFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetRadioCheckFont( aFont );
        aFont = aStyleSettings.GetPushButtonFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetPushButtonFont( aFont );
        aFont = aStyleSettings.GetFieldFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetFieldFont( aFont );
        aFont = aStyleSettings.GetIconFont();
        aFont.SetWeight( WEIGHT_BOLD );
        aStyleSettings.SetIconFont( aFont );
        rSettings.SetStyleSettings( aStyleSettings );
    }
#endif

    if ( bCallHdl )
        GetpApp()->OverrideSystemSettings( rSettings );
}

MouseEvent ImplTranslateMouseEvent( const MouseEvent& rE, Window* pSource, Window* pDest )
{
    Point aPos = pSource->OutputToScreenPixel( rE.GetPosPixel() );
    aPos = pDest->ScreenToOutputPixel( aPos );
    return MouseEvent( aPos, rE.GetClicks(), rE.GetMode(), rE.GetButtons(), rE.GetModifier() );
}

CommandEvent ImplTranslateCommandEvent( const CommandEvent& rCEvt, Window* pSource, Window* pDest )
{
    if ( !rCEvt.IsMouseEvent() )
        return rCEvt;

    Point aPos = pSource->OutputToScreenPixel( rCEvt.GetMousePosPixel() );
    aPos = pDest->ScreenToOutputPixel( aPos );
    return CommandEvent( aPos, rCEvt.GetCommand(), rCEvt.IsMouseEvent(), rCEvt.GetData() );
}

void Window::ImplInitWindowData( WindowType nType )
{
    // We will eventually being removing the inheritance of OutputDevice from Window.
    // It will be replaced with a composition relationship. A Window will use an OutputDevice,
    // it will not *be* an OutputDevice
    mpOutputDevice = (OutputDevice*)this;

    mpWindowImpl = new WindowImpl( nType );

    meOutDevType        = OUTDEV_WINDOW;

    mbEnableRTL         = Application::GetSettings().GetLayoutRTL();         // true: this outdev will be mirrored if RTL window layout (UI mirroring) is globally active
}

static bool ImplDoTiledRendering()
{
#if !HAVE_FEATURE_DESKTOP
    // We do tiled rendering only for iOS at the moment, actually, but
    // let's see what happens if we assume it for Android, too.
    return true;
#else
    // We need some way to know globally if this process will use
    // tiled rendering or not. Or should this be a per-window setting?
    // Or what?
    return false;
#endif
}

static sal_Int32 CountDPIScaleFactor(sal_Int32 nDPI)
{
    sal_Int32 nResult = 1;

#ifndef MACOSX
    // Setting of HiDPI is unfortunately all only a heuristic; and to add
    // insult to an injury, the system is constantly lying to us about
    // the DPI and whatnot
    // eg. fdo#77059 - set the value from which we do consider the
    // screen hi-dpi to greater than 168
    if (nDPI > 168)
        nResult = std::max(sal_Int32(1), (nDPI + 48) / 96);
#else
    (void)nDPI;
#endif

    return nResult;
}

void Window::ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData )
{
    DBG_ASSERT( mpWindowImpl->mbFrame || pParent, "Window::Window(): pParent == NULL" );

    ImplSVData* pSVData = ImplGetSVData();
    Window*     pRealParent = pParent;

    // 3D-Look vererben
    if ( !mpWindowImpl->mbOverlapWin && pParent && (pParent->GetStyle() & WB_3DLOOK) )
        nStyle |= WB_3DLOOK;

    // create border window if necessary
    if ( !mpWindowImpl->mbFrame && !mpWindowImpl->mbBorderWin && !mpWindowImpl->mpBorderWindow
         && (nStyle & (WB_BORDER | WB_SYSTEMCHILDWINDOW) ) )
    {
        sal_uInt16 nBorderTypeStyle = 0;
        if( (nStyle & WB_SYSTEMCHILDWINDOW) )
        {
            // handle WB_SYSTEMCHILDWINDOW
            // these should be analogous to a top level frame; meaning they
            // should have a border window with style BORDERWINDOW_STYLE_FRAME
            // which controls their size
            nBorderTypeStyle |= BORDERWINDOW_STYLE_FRAME;
            nStyle |= WB_BORDER;
        }
        ImplBorderWindow* pBorderWin = new ImplBorderWindow( pParent, nStyle & (WB_BORDER | WB_DIALOGCONTROL | WB_NODIALOGCONTROL | WB_NEEDSFOCUS), nBorderTypeStyle );
        ((Window*)pBorderWin)->mpWindowImpl->mpClientWindow = this;
        pBorderWin->GetBorder( mpWindowImpl->mnLeftBorder, mpWindowImpl->mnTopBorder, mpWindowImpl->mnRightBorder, mpWindowImpl->mnBottomBorder );
        mpWindowImpl->mpBorderWindow  = pBorderWin;
        pParent = mpWindowImpl->mpBorderWindow;
    }
    else if( !mpWindowImpl->mbFrame && ! pParent )
    {
        mpWindowImpl->mbOverlapWin  = true;
        mpWindowImpl->mbFrame = true;
    }

    // insert window in list
    ImplInsertWindow( pParent );
    mpWindowImpl->mnStyle = nStyle;

    // Overlap-Window-Data
    if ( mpWindowImpl->mbOverlapWin )
    {
        mpWindowImpl->mpOverlapData                   = new ImplOverlapData;
        mpWindowImpl->mpOverlapData->mpSaveBackDev    = NULL;
        mpWindowImpl->mpOverlapData->mpSaveBackRgn    = NULL;
        mpWindowImpl->mpOverlapData->mpNextBackWin    = NULL;
        mpWindowImpl->mpOverlapData->mnSaveBackSize   = 0;
        mpWindowImpl->mpOverlapData->mbSaveBack       = false;
        mpWindowImpl->mpOverlapData->mnTopLevel       = 1;
    }

    if( pParent && ! mpWindowImpl->mbFrame )
        mbEnableRTL = Application::GetSettings().GetLayoutRTL();

    // test for frame creation
    if ( mpWindowImpl->mbFrame )
    {
        // create frame
        sal_uLong nFrameStyle = 0;

        if ( nStyle & WB_MOVEABLE )
            nFrameStyle |= SAL_FRAME_STYLE_MOVEABLE;
        if ( nStyle & WB_SIZEABLE )
            nFrameStyle |= SAL_FRAME_STYLE_SIZEABLE;
        if ( nStyle & WB_CLOSEABLE )
            nFrameStyle |= SAL_FRAME_STYLE_CLOSEABLE;
        if ( nStyle & WB_APP )
            nFrameStyle |= SAL_FRAME_STYLE_DEFAULT;
        // check for undecorated floating window
        if( // 1. floating windows that are not moveable/sizeable (only closeable allowed)
            ( !(nFrameStyle & ~SAL_FRAME_STYLE_CLOSEABLE) &&
            ( mpWindowImpl->mbFloatWin || ((GetType() == WINDOW_BORDERWINDOW) && ((ImplBorderWindow*)this)->mbFloatWindow) || (nStyle & WB_SYSTEMFLOATWIN) ) ) ||
            // 2. borderwindows of floaters with ownerdraw decoration
            ( ((GetType() == WINDOW_BORDERWINDOW) && ((ImplBorderWindow*)this)->mbFloatWindow && (nStyle & WB_OWNERDRAWDECORATION) ) ) )
        {
            nFrameStyle = SAL_FRAME_STYLE_FLOAT;
            if( nStyle & WB_OWNERDRAWDECORATION )
                nFrameStyle |= (SAL_FRAME_STYLE_OWNERDRAWDECORATION | SAL_FRAME_STYLE_NOSHADOW);
            if( nStyle & WB_NEEDSFOCUS )
                nFrameStyle |= SAL_FRAME_STYLE_FLOAT_FOCUSABLE;
        }
        else if( mpWindowImpl->mbFloatWin )
            nFrameStyle |= SAL_FRAME_STYLE_TOOLWINDOW;

        if( nStyle & WB_INTROWIN )
            nFrameStyle |= SAL_FRAME_STYLE_INTRO;
        if( nStyle & WB_TOOLTIPWIN )
            nFrameStyle |= SAL_FRAME_STYLE_TOOLTIP;

        if( nStyle & WB_NOSHADOW )
            nFrameStyle |= SAL_FRAME_STYLE_NOSHADOW;

        if( nStyle & WB_SYSTEMCHILDWINDOW )
            nFrameStyle |= SAL_FRAME_STYLE_SYSTEMCHILD;

        switch (mpWindowImpl->mnType)
        {
            case WINDOW_DIALOG:
            case WINDOW_TABDIALOG:
            case WINDOW_MODALDIALOG:
            case WINDOW_MODELESSDIALOG:
            case WINDOW_MESSBOX:
            case WINDOW_INFOBOX:
            case WINDOW_WARNINGBOX:
            case WINDOW_ERRORBOX:
            case WINDOW_QUERYBOX:
                nFrameStyle |= SAL_FRAME_STYLE_DIALOG;
            default:
                break;
        }

        SalFrame* pParentFrame = NULL;
        if ( pParent )
            pParentFrame = pParent->mpWindowImpl->mpFrame;
        SalFrame* pFrame;
        if ( pSystemParentData )
            pFrame = pSVData->mpDefInst->CreateChildFrame( pSystemParentData, nFrameStyle | SAL_FRAME_STYLE_PLUG );
        else
            pFrame = pSVData->mpDefInst->CreateFrame( pParentFrame, nFrameStyle );
        if ( !pFrame )
        {
            // do not abort but throw an exception, may be the current thread terminates anyway (plugin-scenario)
            throw ::com::sun::star::uno::RuntimeException(
                OUString( "Could not create system window!"  ),
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >() );
        }

        pFrame->SetCallback( this, ImplWindowFrameProc );

        // set window frame data
        mpWindowImpl->mpFrameData     = new ImplFrameData;
        mpWindowImpl->mpFrame         = pFrame;
        mpWindowImpl->mpFrameWindow   = this;
        mpWindowImpl->mpOverlapWindow = this;

        // set frame data
        mpWindowImpl->mpFrameData->mpNextFrame        = pSVData->maWinData.mpFirstFrame;
        pSVData->maWinData.mpFirstFrame = this;
        mpWindowImpl->mpFrameData->mpFirstOverlap     = NULL;
        mpWindowImpl->mpFrameData->mpFocusWin         = NULL;
        mpWindowImpl->mpFrameData->mpMouseMoveWin     = NULL;
        mpWindowImpl->mpFrameData->mpMouseDownWin     = NULL;
        mpWindowImpl->mpFrameData->mpFirstBackWin     = NULL;
        mpWindowImpl->mpFrameData->mpFontCollection   = pSVData->maGDIData.mpScreenFontList;
        mpWindowImpl->mpFrameData->mpFontCache        = pSVData->maGDIData.mpScreenFontCache;
        mpWindowImpl->mpFrameData->mnAllSaveBackSize  = 0;
        mpWindowImpl->mpFrameData->mnFocusId          = 0;
        mpWindowImpl->mpFrameData->mnMouseMoveId      = 0;
        mpWindowImpl->mpFrameData->mnLastMouseX       = -1;
        mpWindowImpl->mpFrameData->mnLastMouseY       = -1;
        mpWindowImpl->mpFrameData->mnBeforeLastMouseX = -1;
        mpWindowImpl->mpFrameData->mnBeforeLastMouseY = -1;
        mpWindowImpl->mpFrameData->mnFirstMouseX      = -1;
        mpWindowImpl->mpFrameData->mnFirstMouseY      = -1;
        mpWindowImpl->mpFrameData->mnLastMouseWinX    = -1;
        mpWindowImpl->mpFrameData->mnLastMouseWinY    = -1;
        mpWindowImpl->mpFrameData->mnModalMode        = 0;
        mpWindowImpl->mpFrameData->mnMouseDownTime    = 0;
        mpWindowImpl->mpFrameData->mnClickCount       = 0;
        mpWindowImpl->mpFrameData->mnFirstMouseCode   = 0;
        mpWindowImpl->mpFrameData->mnMouseCode        = 0;
        mpWindowImpl->mpFrameData->mnMouseMode        = 0;
        mpWindowImpl->mpFrameData->meMapUnit          = MAP_PIXEL;
        mpWindowImpl->mpFrameData->mbHasFocus         = false;
        mpWindowImpl->mpFrameData->mbInMouseMove      = false;
        mpWindowImpl->mpFrameData->mbMouseIn          = false;
        mpWindowImpl->mpFrameData->mbStartDragCalled  = false;
        mpWindowImpl->mpFrameData->mbNeedSysWindow    = false;
        mpWindowImpl->mpFrameData->mbMinimized        = false;
        mpWindowImpl->mpFrameData->mbStartFocusState  = false;
        mpWindowImpl->mpFrameData->mbInSysObjFocusHdl = false;
        mpWindowImpl->mpFrameData->mbInSysObjToTopHdl = false;
        mpWindowImpl->mpFrameData->mbSysObjFocus      = false;
        if (!ImplDoTiledRendering())
        {
            mpWindowImpl->mpFrameData->maPaintTimer.SetTimeout( 30 );
            mpWindowImpl->mpFrameData->maPaintTimer.SetTimeoutHdl( LINK( this, Window, ImplHandlePaintHdl ) );
        }
        mpWindowImpl->mpFrameData->maResizeTimer.SetTimeout( 50 );
        mpWindowImpl->mpFrameData->maResizeTimer.SetTimeoutHdl( LINK( this, Window, ImplHandleResizeTimerHdl ) );
        mpWindowImpl->mpFrameData->mbInternalDragGestureRecognizer = false;

        if ( pRealParent && IsTopWindow() )
        {
            ImplWinData* pParentWinData = pRealParent->ImplGetWinData();
            pParentWinData->maTopWindowChildren.push_back( this );
        }
    }

    // init data
    mpWindowImpl->mpRealParent = pRealParent;

    // #99318: make sure fontcache and list is available before call to SetSettings
    mpFontCollection      = mpWindowImpl->mpFrameData->mpFontCollection;
    mpFontCache     = mpWindowImpl->mpFrameData->mpFontCache;

    if ( mpWindowImpl->mbFrame )
    {
        if ( pParent )
        {
            mpWindowImpl->mpFrameData->mnDPIX     = pParent->mpWindowImpl->mpFrameData->mnDPIX;
            mpWindowImpl->mpFrameData->mnDPIY     = pParent->mpWindowImpl->mpFrameData->mnDPIY;
        }
        else
        {
            OutputDevice *pOutDev = GetOutDev();
            if ( pOutDev->AcquireGraphics() )
            {
                mpGraphics->GetResolution( mpWindowImpl->mpFrameData->mnDPIX, mpWindowImpl->mpFrameData->mnDPIY );
            }
        }

        // add ownerdraw decorated frame windows to list in the top-most frame window
        // so they can be hidden on lose focus
        if( nStyle & WB_OWNERDRAWDECORATION )
            ImplGetOwnerDrawList().push_back( this );

        // delay settings initialization until first "real" frame
        // this relies on the IntroWindow not needing any system settings
        if ( !pSVData->maAppData.mbSettingsInit &&
             ! (nStyle & (WB_INTROWIN|WB_DEFAULTWIN))
             )
        {
            // side effect: ImplUpdateGlobalSettings does an ImplGetFrame()->UpdateSettings
            ImplUpdateGlobalSettings( *pSVData->maAppData.mpSettings );
            OutputDevice::SetSettings( *pSVData->maAppData.mpSettings );
            pSVData->maAppData.mbSettingsInit = true;
        }

        // If we create a Window with default size, query this
        // size directly, because we want resize all Controls to
        // the correct size before we display the window
        if ( nStyle & (WB_MOVEABLE | WB_SIZEABLE | WB_APP) )
            mpWindowImpl->mpFrame->GetClientSize( mnOutWidth, mnOutHeight );
    }
    else
    {
        if ( pParent )
        {
            if ( !ImplIsOverlapWindow() )
            {
                mpWindowImpl->mbDisabled          = pParent->mpWindowImpl->mbDisabled;
                mpWindowImpl->mbInputDisabled     = pParent->mpWindowImpl->mbInputDisabled;
                mpWindowImpl->meAlwaysInputMode   = pParent->mpWindowImpl->meAlwaysInputMode;
            }

            OutputDevice::SetSettings( pParent->GetSettings() );
        }

    }

    // setup the scale factor for Hi-DPI displays
    mnDPIScaleFactor = CountDPIScaleFactor(mpWindowImpl->mpFrameData->mnDPIY);

    const StyleSettings& rStyleSettings = mxSettings->GetStyleSettings();
    sal_uInt16 nScreenZoom = rStyleSettings.GetScreenZoom();
    mnDPIX          = (mpWindowImpl->mpFrameData->mnDPIX*nScreenZoom)/100;
    mnDPIY          = (mpWindowImpl->mpFrameData->mnDPIY*nScreenZoom)/100;
    maFont          = rStyleSettings.GetAppFont();
    ImplPointToLogic( maFont );

    if ( nStyle & WB_3DLOOK )
    {
        SetTextColor( rStyleSettings.GetButtonTextColor() );
        SetBackground( Wallpaper( rStyleSettings.GetFaceColor() ) );
    }
    else
    {
        SetTextColor( rStyleSettings.GetWindowTextColor() );
        SetBackground( Wallpaper( rStyleSettings.GetWindowColor() ) );
    }

    ImplUpdatePos();

    // calculate app font res (except for the Intro Window or the default window)
    if ( mpWindowImpl->mbFrame && !pSVData->maGDIData.mnAppFontX && ! (nStyle & (WB_INTROWIN|WB_DEFAULTWIN)) )
        ImplInitAppFontData( this );

    if ( GetAccessibleParentWindow()  && GetParent() != Application::GetDefDialogParent() )
        GetAccessibleParentWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_CHILDCREATED, this );
}

void Window::ImplSetFrameParent( const Window* pParent )
{
    Window* pFrameWindow = ImplGetSVData()->maWinData.mpFirstFrame;
    while( pFrameWindow )
    {
        // search all frames that are children of this window
        // and reparent them
        if( ImplIsRealParentPath( pFrameWindow ) )
        {
            DBG_ASSERT( mpWindowImpl->mpFrame != pFrameWindow->mpWindowImpl->mpFrame, "SetFrameParent to own" );
            DBG_ASSERT( mpWindowImpl->mpFrame, "no frame" );
            SalFrame* pParentFrame = pParent ? pParent->mpWindowImpl->mpFrame : NULL;
            pFrameWindow->mpWindowImpl->mpFrame->SetParent( pParentFrame );
        }
        pFrameWindow = pFrameWindow->mpWindowImpl->mpFrameData->mpNextFrame;
    }
}

void Window::ImplCallResize()
{
    mpWindowImpl->mbCallResize = false;

    if( GetBackground().IsGradient() )
        Invalidate();

    Resize();

    // #88419# Most classes don't call the base class in Resize() and Move(),
    // => Call ImpleResize/Move instead of Resize/Move directly...
    ImplCallEventListeners( VCLEVENT_WINDOW_RESIZE );
}

void Window::ImplCallMove()
{
    mpWindowImpl->mbCallMove = false;

    if( mpWindowImpl->mbFrame )
    {
        // update frame position
        SalFrame *pParentFrame = NULL;
        Window *pParent = ImplGetParent();
        while( pParent )
        {
            if( pParent->mpWindowImpl->mpFrame != mpWindowImpl->mpFrame )
            {
                pParentFrame = pParent->mpWindowImpl->mpFrame;
                break;
            }
            pParent = pParent->GetParent();
        }

        SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
        mpWindowImpl->maPos = Point( g.nX, g.nY );
        if( pParentFrame )
        {
            g = pParentFrame->GetGeometry();
            mpWindowImpl->maPos -= Point( g.nX, g.nY );
        }
        // the client window and and all its subclients have the same position as the borderframe
        // this is important for floating toolbars where the borderwindow is a floating window
        // which has another borderwindow (ie the system floating window)
        Window *pClientWin = mpWindowImpl->mpClientWindow;
        while( pClientWin )
        {
            pClientWin->mpWindowImpl->maPos = mpWindowImpl->maPos;
            pClientWin = pClientWin->mpWindowImpl->mpClientWindow;
        }
    }

    Move();

    ImplCallEventListeners( VCLEVENT_WINDOW_MOVE );
}

ImplWinData* Window::ImplGetWinData() const
{
    if ( !mpWindowImpl->mpWinData )
    {
        static const char* pNoNWF = getenv( "SAL_NO_NWF" );

        ((Window*)this)->mpWindowImpl->mpWinData = new ImplWinData;
        mpWindowImpl->mpWinData->mpExtOldText     = NULL;
        mpWindowImpl->mpWinData->mpExtOldAttrAry  = NULL;
        mpWindowImpl->mpWinData->mpCursorRect     = NULL;
        mpWindowImpl->mpWinData->mnCursorExtWidth = 0;
        mpWindowImpl->mpWinData->mpCompositionCharRects = NULL;
        mpWindowImpl->mpWinData->mnCompositionCharRects = 0;
        mpWindowImpl->mpWinData->mpFocusRect      = NULL;
        mpWindowImpl->mpWinData->mpTrackRect      = NULL;
        mpWindowImpl->mpWinData->mnTrackFlags     = 0;
        mpWindowImpl->mpWinData->mnIsTopWindow  = (sal_uInt16) ~0;  // not initialized yet, 0/1 will indicate TopWindow (see IsTopWindow())
        mpWindowImpl->mpWinData->mbMouseOver      = false;
        mpWindowImpl->mpWinData->mbEnableNativeWidget = (pNoNWF && *pNoNWF) ? sal_False : sal_True; // sal_True: try to draw this control with native theme API
   }

    return mpWindowImpl->mpWinData;
}

SalGraphics* Window::ImplGetFrameGraphics() const
{
    if ( mpWindowImpl->mpFrameWindow->mpGraphics )
    {
        mpWindowImpl->mpFrameWindow->mbInitClipRegion = true;
    }
    else
    {
        OutputDevice *pFrameWinOutDev = mpWindowImpl->mpFrameWindow;
        pFrameWinOutDev->AcquireGraphics();
    }
    mpWindowImpl->mpFrameWindow->mpGraphics->ResetClipRegion();
    return mpWindowImpl->mpFrameWindow->mpGraphics;
}

Window* Window::ImplFindWindow( const Point& rFramePos )
{
    Window* pTempWindow;
    Window* pFindWindow;

    // first check all overlapping windows
    pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
        if ( pFindWindow )
            return pFindWindow;
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }

    // then we check our window
    if ( !mpWindowImpl->mbVisible )
        return NULL;

    sal_uInt16 nHitTest = ImplHitTest( rFramePos );
    if ( nHitTest & WINDOW_HITTEST_INSIDE )
    {
        // and then we check all child windows
        pTempWindow = mpWindowImpl->mpFirstChild;
        while ( pTempWindow )
        {
            pFindWindow = pTempWindow->ImplFindWindow( rFramePos );
            if ( pFindWindow )
                return pFindWindow;
            pTempWindow = pTempWindow->mpWindowImpl->mpNext;
        }

        if ( nHitTest & WINDOW_HITTEST_TRANSPARENT )
            return NULL;
        else
            return this;
    }

    return NULL;
}

sal_uInt16 Window::ImplHitTest( const Point& rFramePos )
{
    Point aFramePos( rFramePos );
    if( ImplIsAntiparallel() )
    {
        // - RTL - re-mirror frame pos at this window
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aFramePos );
    }
    Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
    if ( !aRect.IsInside( aFramePos ) )
        return 0;
    if ( mpWindowImpl->mbWinRegion )
    {
        Point aTempPos = aFramePos;
        aTempPos.X() -= mnOutOffX;
        aTempPos.Y() -= mnOutOffY;
        if ( !mpWindowImpl->maWinRegion.IsInside( aTempPos ) )
            return 0;
    }

    sal_uInt16 nHitTest = WINDOW_HITTEST_INSIDE;
    if ( mpWindowImpl->mbMouseTransparent )
        nHitTest |= WINDOW_HITTEST_TRANSPARENT;
    return nHitTest;
}

bool Window::ImplIsRealParentPath( const Window* pWindow ) const
{
    pWindow = pWindow->GetParent();
    while ( pWindow )
    {
        if ( pWindow == this )
            return true;
        pWindow = pWindow->GetParent();
    }

    return false;
}

bool Window::ImplIsChild( const Window* pWindow, bool bSystemWindow ) const
{
    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return true;
    }
    while ( pWindow );

    return false;
}

bool Window::ImplIsWindowOrChild( const Window* pWindow, bool bSystemWindow ) const
{
    if ( this == pWindow )
        return true;
    return ImplIsChild( pWindow, bSystemWindow );
}

int Window::ImplTestMousePointerSet()
{
    // as soon as mouse is captured, switch mouse-pointer
    if ( IsMouseCaptured() )
        return sal_True;

    // if the mouse is over the window, switch it
    Rectangle aClientRect( Point( 0, 0 ), GetOutputSizePixel() );
    if ( aClientRect.IsInside( GetPointerPosPixel() ) )
        return sal_True;

    return sal_False;
}

PointerStyle Window::ImplGetMousePointer() const
{
    PointerStyle    ePointerStyle;
    bool            bWait = false;

    if ( IsEnabled() && IsInputEnabled() && ! IsInModalMode() )
        ePointerStyle = GetPointer().GetStyle();
    else
        ePointerStyle = POINTER_ARROW;

    const Window* pWindow = this;
    do
    {
        // when the pointer is not visible stop the search, as
        // this status should not be overwritten
        if ( pWindow->mpWindowImpl->mbNoPtrVisible )
            return POINTER_NULL;

        if ( !bWait )
        {
            if ( pWindow->mpWindowImpl->mnWaitCount )
            {
                ePointerStyle = POINTER_WAIT;
                bWait = true;
            }
            else
            {
                if ( pWindow->mpWindowImpl->mbChildPtrOverwrite )
                    ePointerStyle = pWindow->GetPointer().GetStyle();
            }
        }

        if ( pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();
    }
    while ( pWindow );

    return ePointerStyle;
}

void Window::ImplResetReallyVisible()
{
    bool bBecameReallyInvisible = mpWindowImpl->mbReallyVisible;

    mbDevOutput     = false;
    mpWindowImpl->mbReallyVisible = false;
    mpWindowImpl->mbReallyShown   = false;

    // the SHOW/HIDE events serve as indicators to send child creation/destroy events to the access bridge.
    // For this, the data member of the event must not be NULL.
    // Previously, we did this in Window::Show, but there some events got lost in certain situations.
    if( bBecameReallyInvisible && ImplIsAccessibleCandidate() )
        ImplCallEventListeners( VCLEVENT_WINDOW_HIDE, this );
        // TODO. It's kind of a hack that we're re-using the VCLEVENT_WINDOW_HIDE. Normally, we should
        // introduce another event which explicitly triggers the Accessibility implementations.

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
            pWindow->ImplResetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
            pWindow->ImplResetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplSetReallyVisible()
{
    // #i43594# it is possible that INITSHOW was never send, because the visibility state changed between
    // ImplCallInitShow() and ImplSetReallyVisible() when called from Show()
    // mbReallyShown is a useful indicator
    if( !mpWindowImpl->mbReallyShown )
        ImplCallInitShow();

    bool bBecameReallyVisible = !mpWindowImpl->mbReallyVisible;

    mbDevOutput     = true;
    mpWindowImpl->mbReallyVisible = true;
    mpWindowImpl->mbReallyShown   = true;

    // the SHOW/HIDE events serve as indicators to send child creation/destroy events to the access bridge.
    // For this, the data member of the event must not be NULL.
    // Previously, we did this in Window::Show, but there some events got lost in certain situations. Now
    // we're doing it when the visibility really changes
    if( bBecameReallyVisible && ImplIsAccessibleCandidate() )
        ImplCallEventListeners( VCLEVENT_WINDOW_SHOW, this );
        // TODO. It's kind of a hack that we're re-using the VCLEVENT_WINDOW_SHOW. Normally, we should
        // introduce another event which explicitly triggers the Accessibility implementations.

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplSetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplSetReallyVisible();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplCallInitShow()
{
    mpWindowImpl->mbReallyShown   = true;
    mpWindowImpl->mbInInitShow    = true;
    StateChanged( STATE_CHANGE_INITSHOW );
    mpWindowImpl->mbInInitShow    = false;

    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplCallInitShow();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }

    pWindow = mpWindowImpl->mpFirstChild;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbVisible )
            pWindow->ImplCallInitShow();
        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplAddDel( ImplDelData* pDel ) // TODO: make "const" when incompatiblity ok
{
    DBG_ASSERT( !pDel->mpWindow, "Window::ImplAddDel(): cannot add ImplDelData twice !" );
    if( !pDel->mpWindow )
    {
        pDel->mpWindow = this;  // #112873# store ref to this window, so pDel can remove itself
        pDel->mpNext = mpWindowImpl->mpFirstDel;
        mpWindowImpl->mpFirstDel = pDel;
    }
}

void Window::ImplRemoveDel( ImplDelData* pDel ) // TODO: make "const" when incompatiblity ok
{
    pDel->mpWindow = NULL;      // #112873# pDel is not associated with a Window anymore
    if ( mpWindowImpl->mpFirstDel == pDel )
        mpWindowImpl->mpFirstDel = pDel->mpNext;
    else
    {
        ImplDelData* pData = mpWindowImpl->mpFirstDel;
        while ( pData->mpNext != pDel )
            pData = pData->mpNext;
        pData->mpNext = pDel->mpNext;
    }
}

void Window::ImplInitResolutionSettings()
{
    // recalculate AppFont-resolution and DPI-resolution
    if ( mpWindowImpl->mbFrame )
    {
        const StyleSettings& rStyleSettings = mxSettings->GetStyleSettings();
        sal_uInt16 nScreenZoom = rStyleSettings.GetScreenZoom();
        mnDPIX = (mpWindowImpl->mpFrameData->mnDPIX*nScreenZoom)/100;
        mnDPIY = (mpWindowImpl->mpFrameData->mnDPIY*nScreenZoom)/100;

        // setup the scale factor for Hi-DPI displays
        mnDPIScaleFactor = CountDPIScaleFactor(mpWindowImpl->mpFrameData->mnDPIY);
        SetPointFont( rStyleSettings.GetAppFont() );
    }
    else if ( mpWindowImpl->mpParent )
    {
        mnDPIX  = mpWindowImpl->mpParent->mnDPIX;
        mnDPIY  = mpWindowImpl->mpParent->mnDPIY;
        mnDPIScaleFactor = mpWindowImpl->mpParent->mnDPIScaleFactor;
    }

    // update the recalculated values for logical units
    // and also tools belonging to the values
    if ( IsMapMode() )
    {
        MapMode aMapMode = GetMapMode();
        SetMapMode();
        SetMapMode( aMapMode );
    }
}

void Window::ImplPointToLogic( Font& rFont ) const
{
    Size    aSize = rFont.GetSize();
    sal_uInt16  nScreenFontZoom = mxSettings->GetStyleSettings().GetScreenFontZoom();

    if ( aSize.Width() )
    {
        aSize.Width() *= mpWindowImpl->mpFrameData->mnDPIX;
        aSize.Width() += 72/2;
        aSize.Width() /= 72;
        aSize.Width() *= nScreenFontZoom;
        aSize.Width() /= 100;
    }
    aSize.Height() *= mpWindowImpl->mpFrameData->mnDPIY;
    aSize.Height() += 72/2;
    aSize.Height() /= 72;
    aSize.Height() *= nScreenFontZoom;
    aSize.Height() /= 100;

    if ( IsMapModeEnabled() )
        aSize = PixelToLogic( aSize );

    rFont.SetSize( aSize );
}

void Window::ImplLogicToPoint( Font& rFont ) const
{
    Size    aSize = rFont.GetSize();
    sal_uInt16  nScreenFontZoom = mxSettings->GetStyleSettings().GetScreenFontZoom();

    if ( IsMapModeEnabled() )
        aSize = LogicToPixel( aSize );

    if ( aSize.Width() )
    {
        aSize.Width() *= 100;
        aSize.Width() /= nScreenFontZoom;
        aSize.Width() *= 72;
        aSize.Width() += mpWindowImpl->mpFrameData->mnDPIX/2;
        aSize.Width() /= mpWindowImpl->mpFrameData->mnDPIX;
    }
    aSize.Height() *= 100;
    aSize.Height() /= nScreenFontZoom;
    aSize.Height() *= 72;
    aSize.Height() += mpWindowImpl->mpFrameData->mnDPIY/2;
    aSize.Height() /= mpWindowImpl->mpFrameData->mnDPIY;

    rFont.SetSize( aSize );
}

void Window::ImplIntersectWindowRegion( Region& rRegion )
{
    rRegion.Intersect( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                  Size( mnOutWidth, mnOutHeight ) ) );
    if ( mpWindowImpl->mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
}

void Window::ImplExcludeWindowRegion( Region& rRegion )
{
    if ( mpWindowImpl->mbWinRegion )
    {
        Point aPoint( mnOutOffX, mnOutOffY );
        Region aRegion( Rectangle( aPoint,
                                   Size( mnOutWidth, mnOutHeight ) ) );
        aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
        rRegion.Exclude( aRegion );
    }
    else
    {
        Point aPoint( mnOutOffX, mnOutOffY );
        rRegion.Exclude( Rectangle( aPoint,
                                    Size( mnOutWidth, mnOutHeight ) ) );
    }
}

void Window::ImplExcludeOverlapWindows( Region& rRegion )
{
    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
        {
            pWindow->ImplExcludeWindowRegion( rRegion );
            pWindow->ImplExcludeOverlapWindows( rRegion );
        }

        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplExcludeOverlapWindows2( Region& rRegion )
{
    if ( mpWindowImpl->mbReallyVisible )
        ImplExcludeWindowRegion( rRegion );

    ImplExcludeOverlapWindows( rRegion );
}

void Window::ImplIntersectAndUnionOverlapWindows( const Region& rInterRegion, Region& rRegion )
{
    Window* pWindow = mpWindowImpl->mpFirstOverlap;
    while ( pWindow )
    {
        if ( pWindow->mpWindowImpl->mbReallyVisible )
        {
            Region aTempRegion( rInterRegion );
            pWindow->ImplIntersectWindowRegion( aTempRegion );
            rRegion.Union( aTempRegion );
            pWindow->ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
        }

        pWindow = pWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplIntersectAndUnionOverlapWindows2( const Region& rInterRegion, Region& rRegion )
{
    if ( mpWindowImpl->mbReallyVisible )
    {
        Region aTempRegion( rInterRegion );
        ImplIntersectWindowRegion( aTempRegion );
        rRegion.Union( aTempRegion );
    }

    ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
}

void Window::ImplCalcOverlapRegionOverlaps( const Region& rInterRegion, Region& rRegion )
{
    // Clip Overlap Siblings
    Window* pStartOverlapWindow;
    if ( !ImplIsOverlapWindow() )
        pStartOverlapWindow = mpWindowImpl->mpOverlapWindow;
    else
        pStartOverlapWindow = this;
    while ( !pStartOverlapWindow->mpWindowImpl->mbFrame )
    {
        Window* pOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpFirstOverlap;
        while ( pOverlapWindow && (pOverlapWindow != pStartOverlapWindow) )
        {
            pOverlapWindow->ImplIntersectAndUnionOverlapWindows2( rInterRegion, rRegion );
            pOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
        }
        pStartOverlapWindow = pStartOverlapWindow->mpWindowImpl->mpOverlapWindow;
    }

    // Clip Child Overlap Windows
    if ( !ImplIsOverlapWindow() )
        mpWindowImpl->mpOverlapWindow->ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
    else
        ImplIntersectAndUnionOverlapWindows( rInterRegion, rRegion );
}

void Window::ImplCalcOverlapRegion( const Rectangle& rSourceRect, Region& rRegion,
                                    bool bChildren, bool bParent, bool bSiblings )
{
    Region  aRegion( rSourceRect );
    if ( mpWindowImpl->mbWinRegion )
        rRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
    Region  aTempRegion;
    Window* pWindow;

    ImplCalcOverlapRegionOverlaps( aRegion, rRegion );

    // Parent-Boundaries
    if ( bParent )
    {
        pWindow = this;
        if ( !ImplIsOverlapWindow() )
        {
            pWindow = ImplGetParent();
            do
            {
                aTempRegion = aRegion;
                pWindow->ImplExcludeWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
                if ( pWindow->ImplIsOverlapWindow() )
                    break;
                pWindow = pWindow->ImplGetParent();
            }
            while ( pWindow );
        }
        if ( pWindow && !pWindow->mpWindowImpl->mbFrame )
        {
            aTempRegion = aRegion;
            aTempRegion.Exclude( Rectangle( Point( 0, 0 ), Size( mpWindowImpl->mpFrameWindow->mnOutWidth, mpWindowImpl->mpFrameWindow->mnOutHeight ) ) );
            rRegion.Union( aTempRegion );
        }
    }

    // Siblings
    if ( bSiblings && !ImplIsOverlapWindow() )
    {
        pWindow = mpWindowImpl->mpParent->mpWindowImpl->mpFirstChild;
        do
        {
            if ( pWindow->mpWindowImpl->mbReallyVisible && (pWindow != this) )
            {
                aTempRegion = aRegion;
                pWindow->ImplIntersectWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
            }
            pWindow = pWindow->mpWindowImpl->mpNext;
        }
        while ( pWindow );
    }

    if ( bChildren )
    {
        pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            if ( pWindow->mpWindowImpl->mbReallyVisible )
            {
                aTempRegion = aRegion;
                pWindow->ImplIntersectWindowRegion( aTempRegion );
                rRegion.Union( aTempRegion );
            }
            pWindow = pWindow->mpWindowImpl->mpNext;
        }
    }
}

class PaintHelper
{
private:
    Window* m_pWindow;
    Region* m_pChildRegion;
    Rectangle m_aSelectionRect;
    Rectangle m_aPaintRect;
    Region m_aPaintRegion;
    sal_uInt16 m_nPaintFlags;
    bool m_bPop;
    bool m_bRestoreCursor;
public:
    PaintHelper(Window *pWindow, sal_uInt16 nPaintFlags);
    void SetPop()
    {
        m_bPop = true;
    }
    void SetPaintRect(const Rectangle& rRect)
    {
        m_aPaintRect = rRect;
    }
    void SetSelectionRect(const Rectangle& rRect)
    {
        m_aSelectionRect = rRect;
    }
    void SetRestoreCursor(bool bRestoreCursor)
    {
        m_bRestoreCursor = bRestoreCursor;
    }
    bool GetRestoreCursor() const
    {
        return m_bRestoreCursor;
    }
    sal_uInt16 GetPaintFlags() const
    {
        return m_nPaintFlags;
    }
    Region& GetPaintRegion()
    {
        return m_aPaintRegion;
    }
    void DoPaint(const Region* pRegion);
    ~PaintHelper();
};

PaintHelper::PaintHelper(Window *pWindow, sal_uInt16 nPaintFlags)
    : m_pWindow(pWindow)
    , m_pChildRegion(NULL)
    , m_nPaintFlags(nPaintFlags)
    , m_bPop(false)
    , m_bRestoreCursor(false)
{
}

void PaintHelper::DoPaint(const Region* pRegion)
{
    WindowImpl* pWindowImpl = m_pWindow->ImplGetWindowImpl();
    Region* pWinChildClipRegion = m_pWindow->ImplGetWinChildClipRegion();
    if ( pWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
        pWindowImpl->maInvalidateRegion = *pWinChildClipRegion;
    else
    {
        if ( pRegion )
            pWindowImpl->maInvalidateRegion.Union( *pRegion );

        if( pWindowImpl->mpWinData && pWindowImpl->mbTrackVisible )
            /* #98602# need to repaint all children within the
           * tracking rectangle, so the following invert
           * operation takes places without traces of the previous
           * one.
           */
            pWindowImpl->maInvalidateRegion.Union( *pWindowImpl->mpWinData->mpTrackRect );

        if ( pWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN )
            m_pChildRegion = new Region( pWindowImpl->maInvalidateRegion );
        pWindowImpl->maInvalidateRegion.Intersect( *pWinChildClipRegion );
    }
    pWindowImpl->mnPaintFlags = 0;
    if ( !pWindowImpl->maInvalidateRegion.IsEmpty() )
    {
        m_pWindow->PushPaintHelper(this);
        m_pWindow->Paint(m_aPaintRect);
    }
}

void Window::PushPaintHelper(PaintHelper *pHelper)
{
    pHelper->SetPop();

    if ( mpWindowImpl->mpCursor )
        pHelper->SetRestoreCursor(mpWindowImpl->mpCursor->ImplSuspend());

    mbInitClipRegion = true;
    mpWindowImpl->mbInPaint = true;

    // restore Paint-Region
    Region &rPaintRegion = pHelper->GetPaintRegion();
    rPaintRegion = mpWindowImpl->maInvalidateRegion;
    Rectangle   aPaintRect = rPaintRegion.GetBoundRect();

    // - RTL - re-mirror paint rect and region at this window
    if( ImplIsAntiparallel() )
    {
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aPaintRect );
        pOutDev->ReMirror( rPaintRegion );
    }
    aPaintRect = ImplDevicePixelToLogic( aPaintRect);
    mpWindowImpl->mpPaintRegion = &rPaintRegion;
    mpWindowImpl->maInvalidateRegion.SetEmpty();

    if ( (pHelper->GetPaintFlags() & IMPL_PAINT_ERASE) && IsBackground() )
    {
        if ( IsClipRegion() )
        {
            Region aOldRegion = GetClipRegion();
            SetClipRegion();
            Erase();
            SetClipRegion( aOldRegion );
        }
        else
            Erase();
    }

    // #98943# trigger drawing of toolbox selection after all childern are painted
    if( mpWindowImpl->mbDrawSelectionBackground )
        pHelper->SetSelectionRect(aPaintRect);
    pHelper->SetPaintRect(aPaintRect);
}

void Window::PopPaintHelper(PaintHelper *pHelper)
{
    if ( mpWindowImpl->mpWinData )
    {
        if ( mpWindowImpl->mbFocusVisible )
            ImplInvertFocus( *(mpWindowImpl->mpWinData->mpFocusRect) );
    }
    mpWindowImpl->mbInPaint = false;
    mbInitClipRegion = true;
    mpWindowImpl->mpPaintRegion = NULL;
    if ( mpWindowImpl->mpCursor )
        mpWindowImpl->mpCursor->ImplResume(pHelper->GetRestoreCursor());
}

PaintHelper::~PaintHelper()
{
    WindowImpl* pWindowImpl = m_pWindow->ImplGetWindowImpl();
    if (m_bPop)
    {
        m_pWindow->PopPaintHelper(this);
    }

    if ( m_nPaintFlags & (IMPL_PAINT_PAINTALLCHILDREN | IMPL_PAINT_PAINTCHILDREN) )
    {
        // Paint from the bottom child window and frontward.
        Window* pTempWindow = pWindowImpl->mpLastChild;
        while ( pTempWindow )
        {
            if ( pTempWindow->mpWindowImpl->mbVisible )
                pTempWindow->ImplCallPaint( m_pChildRegion, m_nPaintFlags );
            pTempWindow = pTempWindow->mpWindowImpl->mpPrev;
        }
    }

    if ( pWindowImpl->mpWinData && pWindowImpl->mbTrackVisible && (pWindowImpl->mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
        /* #98602# need to invert the tracking rect AFTER
        * the children have painted
        */
        m_pWindow->InvertTracking( *(pWindowImpl->mpWinData->mpTrackRect), pWindowImpl->mpWinData->mnTrackFlags );

    // #98943# draw toolbox selection
    if( !m_aSelectionRect.IsEmpty() )
        m_pWindow->DrawSelectionBackground( m_aSelectionRect, 3, false, true, false );

    delete m_pChildRegion;
}

void Window::ImplCallPaint( const Region* pRegion, sal_uInt16 nPaintFlags )
{
    // call PrePaint. PrePaint may add to the invalidate region as well as
    // other parameters used below.
    PrePaint();

    mpWindowImpl->mbPaintFrame = false;

    if ( nPaintFlags & IMPL_PAINT_PAINTALLCHILDREN )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALLCHILDREN | (nPaintFlags & IMPL_PAINT_PAINTALL);
    if ( nPaintFlags & IMPL_PAINT_PAINTCHILDREN )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTCHILDREN;
    if ( nPaintFlags & IMPL_PAINT_ERASE )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( nPaintFlags & IMPL_PAINT_CHECKRTL )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_CHECKRTL;
    if ( !mpWindowImpl->mpFirstChild )
        mpWindowImpl->mnPaintFlags &= ~IMPL_PAINT_PAINTALLCHILDREN;

    if ( mpWindowImpl->mbPaintDisabled )
    {
        if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
            Invalidate( INVALIDATE_NOCHILDREN | INVALIDATE_NOERASE | INVALIDATE_NOTRANSPARENT | INVALIDATE_NOCLIPCHILDREN );
        else if ( pRegion )
            Invalidate( *pRegion, INVALIDATE_NOCHILDREN | INVALIDATE_NOERASE | INVALIDATE_NOTRANSPARENT | INVALIDATE_NOCLIPCHILDREN );
        return;
    }

    nPaintFlags = mpWindowImpl->mnPaintFlags & ~(IMPL_PAINT_PAINT);

    PaintHelper aHelper(this, nPaintFlags);

    if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINT )
        aHelper.DoPaint(pRegion);
    else
        mpWindowImpl->mnPaintFlags = 0;
}

void Window::ImplCallOverlapPaint()
{
    // emit overlapping windows first
    Window* pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->mpWindowImpl->mbReallyVisible )
            pTempWindow->ImplCallOverlapPaint();
        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }

    // only then ourself
    if ( mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDREN) )
    {
        // - RTL - notify ImplCallPaint to check for re-mirroring (CHECKRTL)
        //         because we were called from the Sal layer
        ImplCallPaint( NULL, mpWindowImpl->mnPaintFlags /*| IMPL_PAINT_CHECKRTL */);
    }
}

void Window::ImplPostPaint()
{
    if ( !ImplDoTiledRendering() && !mpWindowImpl->mpFrameData->maPaintTimer.IsActive() )
        mpWindowImpl->mpFrameData->maPaintTimer.Start();
}

IMPL_LINK_NOARG(Window, ImplHandlePaintHdl)
{
    // save paint events until layout is done
    if (!ImplDoTiledRendering() && IsDialog() && static_cast<const Dialog*>(this)->hasPendingLayout())
    {
        mpWindowImpl->mpFrameData->maPaintTimer.Start();
        return 0;
    }

    // save paint events until resizing is done
    if( !ImplDoTiledRendering() &&
        mpWindowImpl->mbFrame && mpWindowImpl->mpFrameData->maResizeTimer.IsActive() )
        mpWindowImpl->mpFrameData->maPaintTimer.Start();
    else if ( mpWindowImpl->mbReallyVisible )
        ImplCallOverlapPaint();
    return 0;
}

IMPL_LINK_NOARG(Window, ImplHandleResizeTimerHdl)
{
    if( mpWindowImpl->mbReallyVisible )
    {
        ImplCallResize();
        if( ImplDoTiledRendering() )
        {
            ImplHandlePaintHdl(NULL);
        }
        else if( mpWindowImpl->mpFrameData->maPaintTimer.IsActive() )
        {
            mpWindowImpl->mpFrameData->maPaintTimer.Stop();
            mpWindowImpl->mpFrameData->maPaintTimer.GetTimeoutHdl().Call( NULL );
        }
    }

    return 0;
}

void Window::ImplInvalidateFrameRegion( const Region* pRegion, sal_uInt16 nFlags )
{
    // set PAINTCHILDREN for all parent windows till the first OverlapWindow
    if ( !ImplIsOverlapWindow() )
    {
        Window* pTempWindow = this;
        sal_uInt16 nTranspPaint = IsPaintTransparent() ? IMPL_PAINT_PAINT : 0;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTCHILDREN )
                break;
            pTempWindow->mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTCHILDREN | nTranspPaint;
            if( ! pTempWindow->IsPaintTransparent() )
                nTranspPaint = 0;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    // set Paint-Flags
    mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINT;
    if ( nFlags & INVALIDATE_CHILDREN )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTALLCHILDREN;
    if ( !(nFlags & INVALIDATE_NOERASE) )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_ERASE;
    if ( !pRegion )
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_PAINTALL;

    // if not everything has to be redrawn, add the region to it
    if ( !(mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL) )
        mpWindowImpl->maInvalidateRegion.Union( *pRegion );

    // Handle transparent windows correctly: invalidate must be done on the first opaque parent
    if( ((IsPaintTransparent() && !(nFlags & INVALIDATE_NOTRANSPARENT)) || (nFlags & INVALIDATE_TRANSPARENT) )
            && ImplGetParent() )
    {
        Window *pParent = ImplGetParent();
        while( pParent && pParent->IsPaintTransparent() )
            pParent = pParent->ImplGetParent();
        if( pParent )
        {
            Region *pChildRegion;
            if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
                // invalidate the whole child window region in the parent
                pChildRegion = ImplGetWinChildClipRegion();
            else
                // invalidate the same region in the parent that has to be repainted in the child
                pChildRegion = &mpWindowImpl->maInvalidateRegion;

            nFlags |= INVALIDATE_CHILDREN;  // paint should also be done on all children
            nFlags &= ~INVALIDATE_NOERASE;  // parent should paint and erase to create proper background
            pParent->ImplInvalidateFrameRegion( pChildRegion, nFlags );
        }
    }
    ImplPostPaint();
}

void Window::ImplInvalidateOverlapFrameRegion( const Region& rRegion )
{
    Region aRegion = rRegion;

    ImplClipBoundaries( aRegion, true, true );
    if ( !aRegion.IsEmpty() )
        ImplInvalidateFrameRegion( &aRegion, INVALIDATE_CHILDREN );

    // now we invalidate the overlapping windows
    Window* pTempWindow = mpWindowImpl->mpFirstOverlap;
    while ( pTempWindow )
    {
        if ( pTempWindow->IsVisible() )
            pTempWindow->ImplInvalidateOverlapFrameRegion( rRegion );

        pTempWindow = pTempWindow->mpWindowImpl->mpNext;
    }
}

void Window::ImplInvalidateParentFrameRegion( Region& rRegion )
{
    if ( mpWindowImpl->mbOverlapWin )
        mpWindowImpl->mpFrameWindow->ImplInvalidateOverlapFrameRegion( rRegion );
    else
    {
        if( ImplGetParent() )
            ImplGetParent()->ImplInvalidateFrameRegion( &rRegion, INVALIDATE_CHILDREN );
    }
}

void Window::ImplInvalidate( const Region* pRegion, sal_uInt16 nFlags )
{

    // reset background storage
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    // check what has to be redrawn
    bool bInvalidateAll = !pRegion;

    // take Transparent-Invalidate into account
    Window* pOpaqueWindow = this;
    if ( (mpWindowImpl->mbPaintTransparent && !(nFlags & INVALIDATE_NOTRANSPARENT)) || (nFlags & INVALIDATE_TRANSPARENT) )
    {
        Window* pTempWindow = pOpaqueWindow->ImplGetParent();
        while ( pTempWindow )
        {
            if ( !pTempWindow->IsPaintTransparent() )
            {
                pOpaqueWindow = pTempWindow;
                nFlags |= INVALIDATE_CHILDREN;
                bInvalidateAll = false;
                break;
            }

            if ( pTempWindow->ImplIsOverlapWindow() )
                break;

            pTempWindow = pTempWindow->ImplGetParent();
        }
    }

    // assemble region
    sal_uInt16 nOrgFlags = nFlags;
    if ( !(nFlags & (INVALIDATE_CHILDREN | INVALIDATE_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= INVALIDATE_NOCHILDREN;
        else
            nFlags |= INVALIDATE_CHILDREN;
    }
    if ( (nFlags & INVALIDATE_NOCHILDREN) && mpWindowImpl->mpFirstChild )
        bInvalidateAll = false;
    if ( bInvalidateAll )
        ImplInvalidateFrameRegion( NULL, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aRegion( aRect );
        if ( pRegion )
        {
            // --- RTL --- remirror region before intersecting it
            if ( ImplIsAntiparallel() )
            {
                const OutputDevice *pOutDev = GetOutDev();

                Region aRgn( *pRegion );
                pOutDev->ReMirror( aRgn );
                aRegion.Intersect( aRgn );
            }
            else
                aRegion.Intersect( *pRegion );
        }
        ImplClipBoundaries( aRegion, true, true );
        if ( nFlags & INVALIDATE_NOCHILDREN )
        {
            nFlags &= ~INVALIDATE_CHILDREN;
            if ( !(nFlags & INVALIDATE_NOCLIPCHILDREN) )
            {
                if ( nOrgFlags & INVALIDATE_NOCHILDREN )
                    ImplClipAllChildren( aRegion );
                else
                {
                    if ( ImplClipChildren( aRegion ) )
                        nFlags |= INVALIDATE_CHILDREN;
                }
            }
        }
        if ( !aRegion.IsEmpty() )
            ImplInvalidateFrameRegion( &aRegion, nFlags );  // transparency is handled here, pOpaqueWindow not required
    }

    if ( nFlags & INVALIDATE_UPDATE )
        pOpaqueWindow->Update();        // start painting at the opaque parent
}

void Window::ImplMoveInvalidateRegion( const Rectangle& rRect,
                                       long nHorzScroll, long nVertScroll,
                                       bool bChildren )
{
    if ( (mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTALL)) == IMPL_PAINT_PAINT )
    {
        Region aTempRegion = mpWindowImpl->maInvalidateRegion;
        aTempRegion.Intersect( rRect );
        aTempRegion.Move( nHorzScroll, nVertScroll );
        mpWindowImpl->maInvalidateRegion.Union( aTempRegion );
    }

    if ( bChildren && (mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTCHILDREN) )
    {
        Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            pWindow->ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, true );
            pWindow = pWindow->mpWindowImpl->mpNext;
        }
    }
}

void Window::ImplMoveAllInvalidateRegions( const Rectangle& rRect,
                                           long nHorzScroll, long nVertScroll,
                                           bool bChildren )
{
    // also shift Paint-Region when paints need processing
    ImplMoveInvalidateRegion( rRect, nHorzScroll, nVertScroll, bChildren );
    // Paint-Region should be shifted, as drawn by the parents
    if ( !ImplIsOverlapWindow() )
    {
        Region  aPaintAllRegion;
        Window* pPaintAllWindow = this;
        do
        {
            pPaintAllWindow = pPaintAllWindow->ImplGetParent();
            if ( pPaintAllWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN )
            {
                if ( pPaintAllWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
                {
                    aPaintAllRegion.SetEmpty();
                    break;
                }
                else
                    aPaintAllRegion.Union( pPaintAllWindow->mpWindowImpl->maInvalidateRegion );
            }
        }
        while ( !pPaintAllWindow->ImplIsOverlapWindow() );
        if ( !aPaintAllRegion.IsEmpty() )
        {
            aPaintAllRegion.Move( nHorzScroll, nVertScroll );
            sal_uInt16 nPaintFlags = 0;
            if ( bChildren )
                mpWindowImpl->mnPaintFlags |= INVALIDATE_CHILDREN;
            ImplInvalidateFrameRegion( &aPaintAllRegion, nPaintFlags );
        }
    }
}

void Window::ImplValidateFrameRegion( const Region* pRegion, sal_uInt16 nFlags )
{
    if ( !pRegion )
        mpWindowImpl->maInvalidateRegion.SetEmpty();
    else
    {
        // when all child windows have to be drawn we need to invalidate them before doing so
        if ( (mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN) && mpWindowImpl->mpFirstChild )
        {
            Region aChildRegion = mpWindowImpl->maInvalidateRegion;
            if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
            {
                Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                aChildRegion = aRect;
            }
            Window* pChild = mpWindowImpl->mpFirstChild;
            while ( pChild )
            {
                pChild->Invalidate( aChildRegion, INVALIDATE_CHILDREN | INVALIDATE_NOTRANSPARENT );
                pChild = pChild->mpWindowImpl->mpNext;
            }
        }
        if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALL )
        {
            Rectangle aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
            mpWindowImpl->maInvalidateRegion = aRect;
        }
        mpWindowImpl->maInvalidateRegion.Exclude( *pRegion );
    }
    mpWindowImpl->mnPaintFlags &= ~IMPL_PAINT_PAINTALL;

    if ( nFlags & VALIDATE_CHILDREN )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->ImplValidateFrameRegion( pRegion, nFlags );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::ImplValidate( const Region* pRegion, sal_uInt16 nFlags )
{
    // assemble region
    bool    bValidateAll = !pRegion;
    sal_uInt16  nOrgFlags = nFlags;
    if ( !(nFlags & (VALIDATE_CHILDREN | VALIDATE_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= VALIDATE_NOCHILDREN;
        else
            nFlags |= VALIDATE_CHILDREN;
    }
    if ( (nFlags & VALIDATE_NOCHILDREN) && mpWindowImpl->mpFirstChild )
        bValidateAll = false;
    if ( bValidateAll )
        ImplValidateFrameRegion( NULL, nFlags );
    else
    {
        Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
        Region      aRegion( aRect );
        if ( pRegion )
            aRegion.Intersect( *pRegion );
        ImplClipBoundaries( aRegion, true, true );
        if ( nFlags & VALIDATE_NOCHILDREN )
        {
            nFlags &= ~VALIDATE_CHILDREN;
            if ( nOrgFlags & VALIDATE_NOCHILDREN )
                ImplClipAllChildren( aRegion );
            else
            {
                if ( ImplClipChildren( aRegion ) )
                    nFlags |= VALIDATE_CHILDREN;
            }
        }
        if ( !aRegion.IsEmpty() )
            ImplValidateFrameRegion( &aRegion, nFlags );
    }
}

void Window::ImplScroll( const Rectangle& rRect,
                         long nHorzScroll, long nVertScroll, sal_uInt16 nFlags )
{
    if ( !IsDeviceOutputNecessary() )
        return;

    nHorzScroll = ImplLogicWidthToDevicePixel( nHorzScroll );
    nVertScroll = ImplLogicHeightToDevicePixel( nVertScroll );

    if ( !nHorzScroll && !nVertScroll )
        return;

    // restore background storage
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    if ( mpWindowImpl->mpCursor )
        mpWindowImpl->mpCursor->ImplSuspend();

    sal_uInt16 nOrgFlags = nFlags;
    if ( !(nFlags & (SCROLL_CHILDREN | SCROLL_NOCHILDREN)) )
    {
        if ( GetStyle() & WB_CLIPCHILDREN )
            nFlags |= SCROLL_NOCHILDREN;
        else
            nFlags |= SCROLL_CHILDREN;
    }

    Region  aInvalidateRegion;
    bool    bScrollChildren = (nFlags & SCROLL_CHILDREN) != 0;
    bool    bErase = (nFlags & SCROLL_NOERASE) == 0;

    if ( !mpWindowImpl->mpFirstChild )
        bScrollChildren = false;

    OutputDevice *pOutDev = GetOutDev();

    // --- RTL --- check if this window requires special action
    bool bReMirror = ( ImplIsAntiparallel() );

    Rectangle aRectMirror( rRect );
    if( bReMirror )
    {
        // --- RTL --- make sure the invalidate region of this window is
        // computed in the same coordinate space as the one from the overlap windows
        pOutDev->ReMirror( aRectMirror );
    }

    // adapt paint areas
    ImplMoveAllInvalidateRegions( aRectMirror, nHorzScroll, nVertScroll, bScrollChildren );

    if ( !(nFlags & SCROLL_NOINVALIDATE) )
    {
        ImplCalcOverlapRegion( aRectMirror, aInvalidateRegion, !bScrollChildren, true, false );

        // --- RTL ---
        // if the scrolling on the device is performed in the opposite direction
        // then move the overlaps in that direction to compute the invalidate region
        // on the correct side, i.e., revert nHorzScroll

        if ( !aInvalidateRegion.IsEmpty() )
        {
            aInvalidateRegion.Move( bReMirror ? -nHorzScroll : nHorzScroll, nVertScroll );
            bErase = true;
        }
        if ( !(nFlags & SCROLL_NOWINDOWINVALIDATE) )
        {
            Rectangle aDestRect( aRectMirror );
            aDestRect.Move( bReMirror ? -nHorzScroll : nHorzScroll, nVertScroll );
            Region aWinInvalidateRegion( aRectMirror );
            aWinInvalidateRegion.Exclude( aDestRect );

            aInvalidateRegion.Union( aWinInvalidateRegion );
        }
    }

    Point aPoint( mnOutOffX, mnOutOffY );
    Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
    if ( nFlags & SCROLL_CLIP )
        aRegion.Intersect( rRect );
    if ( mpWindowImpl->mbWinRegion )
        aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );

    aRegion.Exclude( aInvalidateRegion );

    ImplClipBoundaries( aRegion, false, true );
    if ( !bScrollChildren )
    {
        if ( nOrgFlags & SCROLL_NOCHILDREN )
            ImplClipAllChildren( aRegion );
        else
            ImplClipChildren( aRegion );
    }
    if ( mbClipRegion && (nFlags & SCROLL_USECLIPREGION) )
        aRegion.Intersect( maRegion );
    if ( !aRegion.IsEmpty() )
    {
        if ( mpWindowImpl->mpWinData )
        {
            if ( mpWindowImpl->mbFocusVisible )
                ImplInvertFocus( *(mpWindowImpl->mpWinData->mpFocusRect) );
            if ( mpWindowImpl->mbTrackVisible && (mpWindowImpl->mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
                InvertTracking( *(mpWindowImpl->mpWinData->mpTrackRect), mpWindowImpl->mpWinData->mnTrackFlags );
        }
#ifndef IOS
        // This seems completely unnecessary with tiled rendering, and
        // causes the "AquaSalGraphics::copyArea() for non-layered
        // graphics" message. Presumably we should bypass this on all
        // platforms when dealing with a "window" that uses tiled
        // rendering at the moment. Unclear how to figure that out,
        // though. Also unclear whether we actually could just not
        // create a "frame window", whatever that exactly is, in the
        // tiled rendering case, or at least for platforms where tiles
        // rendering is all there is.

        SalGraphics* pGraphics = ImplGetFrameGraphics();
        if ( pGraphics )
        {
            if( bReMirror )
            {
                // --- RTL --- frame coordinates require re-mirroring
                pOutDev->ReMirror( aRegion );
            }

            pOutDev->SelectClipRegion( aRegion, pGraphics );
            pGraphics->CopyArea( rRect.Left()+nHorzScroll, rRect.Top()+nVertScroll,
                                 rRect.Left(), rRect.Top(),
                                 rRect.GetWidth(), rRect.GetHeight(),
                                 SAL_COPYAREA_WINDOWINVALIDATE, this );
        }
#endif
        if ( mpWindowImpl->mpWinData )
        {
            if ( mpWindowImpl->mbFocusVisible )
                ImplInvertFocus( *(mpWindowImpl->mpWinData->mpFocusRect) );
            if ( mpWindowImpl->mbTrackVisible && (mpWindowImpl->mpWinData->mnTrackFlags & SHOWTRACK_WINDOW) )
                InvertTracking( *(mpWindowImpl->mpWinData->mpTrackRect), mpWindowImpl->mpWinData->mnTrackFlags );
        }
    }

    if ( !aInvalidateRegion.IsEmpty() )
    {
        // --- RTL --- the invalidate region for this windows is already computed in frame coordinates
        // so it has to be re-mirrored before calling the Paint-handler
        mpWindowImpl->mnPaintFlags |= IMPL_PAINT_CHECKRTL;

        sal_uInt16 nPaintFlags = INVALIDATE_CHILDREN;
        if ( !bErase )
            nPaintFlags |= INVALIDATE_NOERASE;
        if ( !bScrollChildren )
        {
            if ( nOrgFlags & SCROLL_NOCHILDREN )
                ImplClipAllChildren( aInvalidateRegion );
            else
                ImplClipChildren( aInvalidateRegion );
        }
        ImplInvalidateFrameRegion( &aInvalidateRegion, nPaintFlags );
    }

    if ( bScrollChildren )
    {
        Window* pWindow = mpWindowImpl->mpFirstChild;
        while ( pWindow )
        {
            Point aPos = pWindow->GetPosPixel();
            aPos += Point( nHorzScroll, nVertScroll );
            pWindow->SetPosPixel( aPos );

            pWindow = pWindow->mpWindowImpl->mpNext;
        }
    }

    if ( nFlags & SCROLL_UPDATE )
        Update();

    if ( mpWindowImpl->mpCursor )
        mpWindowImpl->mpCursor->ImplResume();
}

void Window::ImplUpdateAll( bool bOverlapWindows )
{
    if ( !mpWindowImpl->mbReallyVisible )
        return;

    bool bFlush = false;
    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) )
            bFlush = true;
    }

    // an update changes the OverlapWindow, such that for later paints
    // not too much has to be drawn, if ALLCHILDREN etc. is set
    Window* pWindow = ImplGetFirstOverlapWindow();
    if ( bOverlapWindows )
        pWindow->ImplCallOverlapPaint();
    else
    {
        if ( pWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDREN) )
            pWindow->ImplCallPaint( NULL, pWindow->mpWindowImpl->mnPaintFlags );
    }

    if ( bFlush )
        Flush();
}

void Window::ImplUpdateWindowPtr( Window* pWindow )
{
    if ( mpWindowImpl->mpFrameWindow != pWindow->mpWindowImpl->mpFrameWindow )
    {
        // release graphic
        OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReleaseGraphics();
    }

    mpWindowImpl->mpFrameData     = pWindow->mpWindowImpl->mpFrameData;
    mpWindowImpl->mpFrame         = pWindow->mpWindowImpl->mpFrame;
    mpWindowImpl->mpFrameWindow   = pWindow->mpWindowImpl->mpFrameWindow;
    if ( pWindow->ImplIsOverlapWindow() )
        mpWindowImpl->mpOverlapWindow = pWindow;
    else
        mpWindowImpl->mpOverlapWindow = pWindow->mpWindowImpl->mpOverlapWindow;

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateWindowPtr( pWindow );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

void Window::ImplUpdateWindowPtr()
{
    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateWindowPtr( this );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

void Window::ImplUpdateOverlapWindowPtr( bool bNewFrame )
{
    bool bVisible = IsVisible();
    Show( false );
    ImplRemoveWindow( bNewFrame );
    Window* pRealParent = mpWindowImpl->mpRealParent;
    ImplInsertWindow( ImplGetParent() );
    mpWindowImpl->mpRealParent = pRealParent;
    ImplUpdateWindowPtr();
    if ( ImplUpdatePos() )
        ImplUpdateSysObjPos();

    if ( bNewFrame )
    {
        Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
        while ( pOverlapWindow )
        {
            Window* pNextOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
            pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
            pOverlapWindow = pNextOverlapWindow;
        }
    }

    if ( bVisible )
        Show( true );
}

bool Window::ImplUpdatePos()
{
    bool bSysChild = false;

    if ( ImplIsOverlapWindow() )
    {
        mnOutOffX  = mpWindowImpl->mnX;
        mnOutOffY  = mpWindowImpl->mnY;
    }
    else
    {
        Window* pParent = ImplGetParent();

        mnOutOffX  = mpWindowImpl->mnX + pParent->mnOutOffX;
        mnOutOffY  = mpWindowImpl->mnY + pParent->mnOutOffY;
    }

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        if ( pChild->ImplUpdatePos() )
            bSysChild = true;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    if ( mpWindowImpl->mpSysObj )
        bSysChild = true;

    return bSysChild;
}

void Window::ImplUpdateSysObjPos()
{
    if ( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->SetPosSize( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight );

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->ImplUpdateSysObjPos();
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

void Window::ImplPosSizeWindow( long nX, long nY,
                                long nWidth, long nHeight, sal_uInt16 nFlags )
{
    bool    bNewPos         = false;
    bool    bNewSize        = false;
    bool    bCopyBits       = false;
    long    nOldOutOffX     = mnOutOffX;
    long    nOldOutOffY     = mnOutOffY;
    long    nOldOutWidth    = mnOutWidth;
    long    nOldOutHeight   = mnOutHeight;
    Region* pOverlapRegion  = NULL;
    Region* pOldRegion      = NULL;

    if ( IsReallyVisible() )
    {
        if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
            ImplInvalidateAllOverlapBackgrounds();

        Rectangle aOldWinRect( Point( nOldOutOffX, nOldOutOffY ),
                               Size( nOldOutWidth, nOldOutHeight ) );
        pOldRegion = new Region( aOldWinRect );
        if ( mpWindowImpl->mbWinRegion )
            pOldRegion->Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );

        if ( mnOutWidth && mnOutHeight && !mpWindowImpl->mbPaintTransparent &&
             !mpWindowImpl->mbInitWinClipRegion && !mpWindowImpl->maWinClipRegion.IsEmpty() &&
             !HasPaintEvent() )
            bCopyBits = true;
    }

    bool bnXRecycled = false; // avoid duplicate mirroring in RTL case
    if ( nFlags & WINDOW_POSSIZE_WIDTH )
    {
        if(!( nFlags & WINDOW_POSSIZE_X ))
        {
            nX = mpWindowImpl->mnX;
            nFlags |= WINDOW_POSSIZE_X;
            bnXRecycled = true; // we're using a mnX which was already mirrored in RTL case
        }

        if ( nWidth < 0 )
            nWidth = 0;
        if ( nWidth != mnOutWidth )
        {
            mnOutWidth = nWidth;
            bNewSize = true;
            bCopyBits = false;
        }
    }
    if ( nFlags & WINDOW_POSSIZE_HEIGHT )
    {
        if ( nHeight < 0 )
            nHeight = 0;
        if ( nHeight != mnOutHeight )
        {
            mnOutHeight = nHeight;
            bNewSize = true;
            bCopyBits = false;
        }
    }

    if ( nFlags & WINDOW_POSSIZE_X )
    {
        long nOrgX = nX;
        // --- RTL ---  (compare the screen coordinates)
        Point aPtDev( Point( nX+mnOutOffX, 0 ) );
        OutputDevice *pOutDev = GetOutDev();
        if( pOutDev->HasMirroredGraphics() )
        {
            mpGraphics->mirror( aPtDev.X(), this );

            // #106948# always mirror our pos if our parent is not mirroring, even
            // if we are also not mirroring
            // --- RTL --- check if parent is in different coordinates
            if( !bnXRecycled && mpWindowImpl->mpParent && !mpWindowImpl->mpParent->mpWindowImpl->mbFrame && mpWindowImpl->mpParent->ImplIsAntiparallel() )
            {
                // --- RTL --- (re-mirror at parent window)
                nX = mpWindowImpl->mpParent->mnOutWidth - mnOutWidth - nX;
            }
            /* #i99166# An LTR window in RTL UI that gets sized only would be
               expected to not moved its upper left point
            */
            if( bnXRecycled )
            {
                if( ImplIsAntiparallel() )
                {
                    aPtDev.X() = mpWindowImpl->mnAbsScreenX;
                    nOrgX = mpWindowImpl->maPos.X();
                }
            }
        }
        else if( !bnXRecycled && mpWindowImpl->mpParent && !mpWindowImpl->mpParent->mpWindowImpl->mbFrame && mpWindowImpl->mpParent->ImplIsAntiparallel() )
        {
            // mirrored window in LTR UI
            {
                // --- RTL --- (re-mirror at parent window)
                nX = mpWindowImpl->mpParent->mnOutWidth - mnOutWidth - nX;
            }
        }

        // check maPos as well, as it could have been changed for client windows (ImplCallMove())
        if ( mpWindowImpl->mnAbsScreenX != aPtDev.X() || nX != mpWindowImpl->mnX || nOrgX != mpWindowImpl->maPos.X() )
        {
            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, false, true, true );
            }
            mpWindowImpl->mnX = nX;
            mpWindowImpl->maPos.X() = nOrgX;
            mpWindowImpl->mnAbsScreenX = aPtDev.X();    // --- RTL --- (store real screen pos)
            bNewPos = true;
        }
    }
    if ( nFlags & WINDOW_POSSIZE_Y )
    {
        // check maPos as well, as it could have been changed for client windows (ImplCallMove())
        if ( nY != mpWindowImpl->mnY || nY != mpWindowImpl->maPos.Y() )
        {
            if ( bCopyBits && !pOverlapRegion )
            {
                pOverlapRegion = new Region();
                ImplCalcOverlapRegion( Rectangle( Point( mnOutOffX, mnOutOffY ),
                                                  Size( mnOutWidth, mnOutHeight ) ),
                                       *pOverlapRegion, false, true, true );
            }
            mpWindowImpl->mnY = nY;
            mpWindowImpl->maPos.Y() = nY;
            bNewPos = true;
        }
    }

    if ( bNewPos || bNewSize )
    {
        bool bUpdateSysObjPos = false;
        if ( bNewPos )
            bUpdateSysObjPos = ImplUpdatePos();

        // the borderwindow always specifies the position for its client window
        if ( mpWindowImpl->mpBorderWindow )
            mpWindowImpl->maPos = mpWindowImpl->mpBorderWindow->mpWindowImpl->maPos;

        if ( mpWindowImpl->mpClientWindow )
        {
            mpWindowImpl->mpClientWindow->ImplPosSizeWindow( mpWindowImpl->mpClientWindow->mpWindowImpl->mnLeftBorder,
                                               mpWindowImpl->mpClientWindow->mpWindowImpl->mnTopBorder,
                                               mnOutWidth-mpWindowImpl->mpClientWindow->mpWindowImpl->mnLeftBorder-mpWindowImpl->mpClientWindow->mpWindowImpl->mnRightBorder,
                                               mnOutHeight-mpWindowImpl->mpClientWindow->mpWindowImpl->mnTopBorder-mpWindowImpl->mpClientWindow->mpWindowImpl->mnBottomBorder,
                                               WINDOW_POSSIZE_X | WINDOW_POSSIZE_Y |
                                               WINDOW_POSSIZE_WIDTH | WINDOW_POSSIZE_HEIGHT );
            // Wenn wir ein ClientWindow haben, dann hat dieses fuer die
            // Applikation auch die Position des FloatingWindows
            mpWindowImpl->mpClientWindow->mpWindowImpl->maPos = mpWindowImpl->maPos;
            if ( bNewPos )
            {
                if ( mpWindowImpl->mpClientWindow->IsVisible() )
                {
                    mpWindowImpl->mpClientWindow->ImplCallMove();
                }
                else
                {
                    mpWindowImpl->mpClientWindow->mpWindowImpl->mbCallMove = true;
                }
            }
        }

        // Move()/Resize() will be called only for Show(), such that
        // at least one is called before Show()
        if ( IsVisible() )
        {
            if ( bNewPos )
            {
                ImplCallMove();
            }
            if ( bNewSize )
            {
                ImplCallResize();
            }
        }
        else
        {
            if ( bNewPos )
                mpWindowImpl->mbCallMove = true;
            if ( bNewSize )
                mpWindowImpl->mbCallResize = true;
        }

        bool bUpdateSysObjClip = false;
        if ( IsReallyVisible() )
        {
            if ( bNewPos || bNewSize )
            {
                // reset background storage
                if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mpSaveBackDev )
                    ImplDeleteOverlapBackground();
                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                // set Clip-Flag
                bUpdateSysObjClip = !ImplSetClipFlag( true );
            }

            // invalidate window content ?
            if ( bNewPos || (mnOutWidth > nOldOutWidth) || (mnOutHeight > nOldOutHeight) )
            {
                if ( bNewPos )
                {
                    bool bInvalidate = false;
                    bool bParentPaint = true;
                    if ( !ImplIsOverlapWindow() )
                        bParentPaint = mpWindowImpl->mpParent->IsPaintEnabled();
                    if ( bCopyBits && bParentPaint && !HasPaintEvent() )
                    {
                        Point aPoint( mnOutOffX, mnOutOffY );
                        Region aRegion( Rectangle( aPoint,
                                                   Size( mnOutWidth, mnOutHeight ) ) );
                        if ( mpWindowImpl->mbWinRegion )
                            aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
                        ImplClipBoundaries( aRegion, false, true );
                        if ( !pOverlapRegion->IsEmpty() )
                        {
                            pOverlapRegion->Move( mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY );
                            aRegion.Exclude( *pOverlapRegion );
                        }
                        if ( !aRegion.IsEmpty() )
                        {
                            // adapt Paint areas
                            ImplMoveAllInvalidateRegions( Rectangle( Point( nOldOutOffX, nOldOutOffY ),
                                                                     Size( nOldOutWidth, nOldOutHeight ) ),
                                                          mnOutOffX-nOldOutOffX, mnOutOffY-nOldOutOffY,
                                                          true );
                            SalGraphics* pGraphics = ImplGetFrameGraphics();
                            if ( pGraphics )
                            {

                                OutputDevice *pOutDev = GetOutDev();
                                const bool bSelectClipRegion = pOutDev->SelectClipRegion( aRegion, pGraphics );
                                if ( bSelectClipRegion )
                                {
                                    pGraphics->CopyArea( mnOutOffX, mnOutOffY,
                                                         nOldOutOffX, nOldOutOffY,
                                                         nOldOutWidth, nOldOutHeight,
                                                         SAL_COPYAREA_WINDOWINVALIDATE, this );
                                }
                                else
                                    bInvalidate = true;
                            }
                            else
                                bInvalidate = true;
                            if ( !bInvalidate )
                            {
                                if ( !pOverlapRegion->IsEmpty() )
                                    ImplInvalidateFrameRegion( pOverlapRegion, INVALIDATE_CHILDREN );
                            }
                        }
                        else
                            bInvalidate = true;
                    }
                    else
                        bInvalidate = true;
                    if ( bInvalidate )
                        ImplInvalidateFrameRegion( NULL, INVALIDATE_CHILDREN );
                }
                else
                {
                    Point aPoint( mnOutOffX, mnOutOffY );
                    Region aRegion( Rectangle( aPoint,
                                               Size( mnOutWidth, mnOutHeight ) ) );
                    aRegion.Exclude( *pOldRegion );
                    if ( mpWindowImpl->mbWinRegion )
                        aRegion.Intersect( ImplPixelToDevicePixel( mpWindowImpl->maWinRegion ) );
                    ImplClipBoundaries( aRegion, false, true );
                    if ( !aRegion.IsEmpty() )
                        ImplInvalidateFrameRegion( &aRegion, INVALIDATE_CHILDREN );
                }
            }

            // invalidate Parent or Overlaps
            if ( bNewPos ||
                 (mnOutWidth < nOldOutWidth) || (mnOutHeight < nOldOutHeight) )
            {
                Region aRegion( *pOldRegion );
                if ( !mpWindowImpl->mbPaintTransparent )
                    ImplExcludeWindowRegion( aRegion );
                ImplClipBoundaries( aRegion, false, true );
                if ( !aRegion.IsEmpty() && !mpWindowImpl->mpBorderWindow )
                    ImplInvalidateParentFrameRegion( aRegion );
            }
        }

        // adapt system objects
        if ( bUpdateSysObjClip )
            ImplUpdateSysObjClip();
        if ( bUpdateSysObjPos )
            ImplUpdateSysObjPos();
        if ( bNewSize && mpWindowImpl->mpSysObj )
            mpWindowImpl->mpSysObj->SetPosSize( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight );
    }

    delete pOverlapRegion;
    delete pOldRegion;
}

void Window::ImplCallMouseMove( sal_uInt16 nMouseCode, bool bModChanged )
{
    if ( mpWindowImpl->mpFrameData->mbMouseIn && mpWindowImpl->mpFrameWindow->mpWindowImpl->mbReallyVisible )
    {
        sal_uLong   nTime   = Time::GetSystemTicks();
        long    nX      = mpWindowImpl->mpFrameData->mnLastMouseX;
        long    nY      = mpWindowImpl->mpFrameData->mnLastMouseY;
        sal_uInt16  nCode   = nMouseCode;
        sal_uInt16  nMode   = mpWindowImpl->mpFrameData->mnMouseMode;
        bool    bLeave;
        // check for MouseLeave
        if ( ((nX < 0) || (nY < 0) ||
              (nX >= mpWindowImpl->mpFrameWindow->mnOutWidth) ||
              (nY >= mpWindowImpl->mpFrameWindow->mnOutHeight)) &&
             !ImplGetSVData()->maWinData.mpCaptureWin )
            bLeave = true;
        else
            bLeave = false;
        nMode |= MOUSE_SYNTHETIC;
        if ( bModChanged )
            nMode |= MOUSE_MODIFIERCHANGED;
        ImplHandleMouseEvent( mpWindowImpl->mpFrameWindow, EVENT_MOUSEMOVE, bLeave, nX, nY, nTime, nCode, nMode );
    }
}

void Window::ImplGenerateMouseMove()
{
    if ( !mpWindowImpl->mpFrameData->mnMouseMoveId )
        mpWindowImpl->mpFrameData->mnMouseMoveId = Application::PostUserEvent( LINK( mpWindowImpl->mpFrameWindow, Window, ImplGenerateMouseMoveHdl ) );
}

IMPL_LINK_NOARG(Window, ImplGenerateMouseMoveHdl)
{
    mpWindowImpl->mpFrameData->mnMouseMoveId = 0;
    Window* pCaptureWin = ImplGetSVData()->maWinData.mpCaptureWin;
    if( ! pCaptureWin ||
        (pCaptureWin->mpWindowImpl && pCaptureWin->mpWindowImpl->mpFrame == mpWindowImpl->mpFrame)
    )
    {
        ImplCallMouseMove( mpWindowImpl->mpFrameData->mnMouseCode );
    }
    return 0;
}

void Window::ImplInvertFocus( const Rectangle& rRect )
{
    InvertTracking( rRect, SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
}

void Window::ImplCallFocusChangeActivate( Window* pNewOverlapWindow,
                                          Window* pOldOverlapWindow )
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pNewRealWindow;
    Window*     pOldRealWindow;
    bool bCallActivate = true;
    bool bCallDeactivate = true;

    pOldRealWindow = pOldOverlapWindow->ImplGetWindow();
    pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
    if ( (pOldRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
         pOldRealWindow->GetActivateMode() )
    {
        if ( (pNewRealWindow->GetType() == WINDOW_FLOATINGWINDOW) &&
             !pNewRealWindow->GetActivateMode() )
        {
            pSVData->maWinData.mpLastDeacWin = pOldOverlapWindow;
            bCallDeactivate = false;
        }
    }
    else if ( (pNewRealWindow->GetType() != WINDOW_FLOATINGWINDOW) ||
              pNewRealWindow->GetActivateMode() )
    {
        if ( pSVData->maWinData.mpLastDeacWin )
        {
            if ( pSVData->maWinData.mpLastDeacWin == pNewOverlapWindow )
                bCallActivate = false;
            else
            {
                Window* pLastRealWindow = pSVData->maWinData.mpLastDeacWin->ImplGetWindow();
                pSVData->maWinData.mpLastDeacWin->mpWindowImpl->mbActive = false;
                pSVData->maWinData.mpLastDeacWin->Deactivate();
                if ( pLastRealWindow != pSVData->maWinData.mpLastDeacWin )
                {
                    pLastRealWindow->mpWindowImpl->mbActive = true;
                    pLastRealWindow->Activate();
                }
            }
            pSVData->maWinData.mpLastDeacWin = NULL;
        }
    }

    if ( bCallDeactivate )
    {
        if( pOldOverlapWindow->mpWindowImpl->mbActive )
        {
            pOldOverlapWindow->mpWindowImpl->mbActive = false;
            pOldOverlapWindow->Deactivate();
        }
        if ( pOldRealWindow != pOldOverlapWindow )
        {
            if( pOldRealWindow->mpWindowImpl->mbActive )
            {
                pOldRealWindow->mpWindowImpl->mbActive = false;
                pOldRealWindow->Deactivate();
            }
        }
    }
    if ( bCallActivate && ! pNewOverlapWindow->mpWindowImpl->mbActive )
    {
        if( ! pNewOverlapWindow->mpWindowImpl->mbActive )
        {
            pNewOverlapWindow->mpWindowImpl->mbActive = true;
            pNewOverlapWindow->Activate();
        }
        if ( pNewRealWindow != pNewOverlapWindow )
        {
            if( ! pNewRealWindow->mpWindowImpl->mbActive )
            {
                pNewRealWindow->mpWindowImpl->mbActive = true;
                pNewRealWindow->Activate();
            }
        }
    }
}

static bool IsWindowFocused(const WindowImpl& rWinImpl)
{
    if (rWinImpl.mpSysObj)
        return true;

    if (rWinImpl.mpFrameData->mbHasFocus)
        return true;

    if (rWinImpl.mbFakeFocusSet)
        return true;

    return false;
}

void Window::ImplGrabFocus( sal_uInt16 nFlags )
{
    // #143570# no focus for destructing windows
    if( mpWindowImpl->mbInDtor )
        return;

    // some event listeners do really bad stuff
    // => prepare for the worst
    ImplDelData aDogTag( this );

    // Currently the client window should always get the focus
    // Should the border window at some point be focusable
    // we need to change all GrabFocus() instances in VCL,
    // e.g. in ToTop()

    if ( mpWindowImpl->mpClientWindow )
    {
        // For a lack of design we need a little hack here to
        // ensure that dialogs on close pass the focus back to
        // the correct window
        if ( mpWindowImpl->mpLastFocusWindow && (mpWindowImpl->mpLastFocusWindow != this) &&
             !(mpWindowImpl->mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) &&
             mpWindowImpl->mpLastFocusWindow->IsEnabled() &&
             mpWindowImpl->mpLastFocusWindow->IsInputEnabled() &&
             ! mpWindowImpl->mpLastFocusWindow->IsInModalMode()
             )
            mpWindowImpl->mpLastFocusWindow->GrabFocus();
        else
            mpWindowImpl->mpClientWindow->GrabFocus();
        return;
    }
    else if ( mpWindowImpl->mbFrame )
    {
        // For a lack of design we need a little hack here to
        // ensure that dialogs on close pass the focus back to
        // the correct window
        if ( mpWindowImpl->mpLastFocusWindow && (mpWindowImpl->mpLastFocusWindow != this) &&
             !(mpWindowImpl->mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) &&
             mpWindowImpl->mpLastFocusWindow->IsEnabled() &&
             mpWindowImpl->mpLastFocusWindow->IsInputEnabled() &&
             ! mpWindowImpl->mpLastFocusWindow->IsInModalMode()
             )
        {
            mpWindowImpl->mpLastFocusWindow->GrabFocus();
            return;
        }
    }

    // If the Window is disabled, then we don't change the focus
    if ( !IsEnabled() || !IsInputEnabled() || IsInModalNonRefMode() )
        return;

    // we only need to set the focus if it is not already set
    // note: if some other frame is waiting for an asynchrounous focus event
    // we also have to post an asynchronous focus event for this frame
    // which is done using ToTop
    ImplSVData* pSVData = ImplGetSVData();

    bool bAsyncFocusWaiting = false;
    Window *pFrame = pSVData->maWinData.mpFirstFrame;
    while( pFrame  )
    {
        if( pFrame != mpWindowImpl->mpFrameWindow && pFrame->mpWindowImpl->mpFrameData->mnFocusId )
        {
            bAsyncFocusWaiting = true;
            break;
        }
        pFrame = pFrame->mpWindowImpl->mpFrameData->mpNextFrame;
    }

    bool bHasFocus = IsWindowFocused(*mpWindowImpl);

    bool bMustNotGrabFocus = false;
    // #100242#, check parent hierarchy if some floater prohibits grab focus

    Window *pParent = this;
    while( pParent )
    {
        // #102158#, ignore grabfocus only if the floating parent grabs keyboard focus by itself (GrabsFocus())
        // otherwise we cannot set the focus in a floating toolbox
        if( ( (pParent->mpWindowImpl->mbFloatWin && ((FloatingWindow*)pParent)->GrabsFocus()) || ( pParent->GetStyle() & WB_SYSTEMFLOATWIN ) ) && !( pParent->GetStyle() & WB_MOVEABLE ) )
        {
            bMustNotGrabFocus = true;
            break;
        }
        pParent = pParent->mpWindowImpl->mpParent;
    }

    if ( ( pSVData->maWinData.mpFocusWin != this && ! mpWindowImpl->mbInDtor ) || ( bAsyncFocusWaiting && !bHasFocus && !bMustNotGrabFocus ) )
    {
        // EndExtTextInput if it is not the same window
        if ( pSVData->maWinData.mpExtTextInputWin &&
             (pSVData->maWinData.mpExtTextInputWin != this) )
            pSVData->maWinData.mpExtTextInputWin->EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );

        // mark this windows as the last FocusWindow
        Window* pOverlapWindow = ImplGetFirstOverlapWindow();
        pOverlapWindow->mpWindowImpl->mpLastFocusWindow = this;
        mpWindowImpl->mpFrameData->mpFocusWin = this;

        if( !bHasFocus )
        {
            // menu windows never get the system focus
            // the application will keep the focus
            if( bMustNotGrabFocus )
                return;
            else
            {
                // here we already switch focus as ToTop()
                // should not give focus to another window
                //DBG_WARNING( "Window::GrabFocus() - Frame doesn't have the focus" );
                mpWindowImpl->mpFrame->ToTop( SAL_FRAME_TOTOP_GRABFOCUS | SAL_FRAME_TOTOP_GRABFOCUS_ONLY );
                return;
            }
        }

        Window* pOldFocusWindow = pSVData->maWinData.mpFocusWin;
        ImplDelData aOldFocusDel( pOldFocusWindow );

        pSVData->maWinData.mpFocusWin = this;

        if ( pOldFocusWindow )
        {
            // Cursor hidden
            if ( pOldFocusWindow->mpWindowImpl->mpCursor )
                pOldFocusWindow->mpWindowImpl->mpCursor->ImplHide( true );
        }

        // !!!!! due to old SV-Office Activate/Deactivate handling
        // !!!!! first as before
        if ( pOldFocusWindow )
        {
            // remember Focus
            Window* pOldOverlapWindow = pOldFocusWindow->ImplGetFirstOverlapWindow();
            Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
            if ( pOldOverlapWindow != pNewOverlapWindow )
                ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
        }
        else
        {
            Window* pNewOverlapWindow = ImplGetFirstOverlapWindow();
            Window* pNewRealWindow = pNewOverlapWindow->ImplGetWindow();
            pNewOverlapWindow->mpWindowImpl->mbActive = true;
            pNewOverlapWindow->Activate();
            if ( pNewRealWindow != pNewOverlapWindow )
            {
                pNewRealWindow->mpWindowImpl->mbActive = true;
                pNewRealWindow->Activate();
            }
        }

        // call Get- and LoseFocus
        if ( pOldFocusWindow && ! aOldFocusDel.IsDead() )
        {
            if ( pOldFocusWindow->IsTracking() &&
                 (pSVData->maWinData.mnTrackFlags & STARTTRACK_FOCUSCANCEL) )
                pOldFocusWindow->EndTracking( ENDTRACK_CANCEL | ENDTRACK_FOCUS );
            NotifyEvent aNEvt( EVENT_LOSEFOCUS, pOldFocusWindow );
            if ( !ImplCallPreNotify( aNEvt ) )
                pOldFocusWindow->LoseFocus();
            pOldFocusWindow->ImplCallDeactivateListeners( this );
        }

        if ( pSVData->maWinData.mpFocusWin == this )
        {
            if ( mpWindowImpl->mpSysObj )
            {
                mpWindowImpl->mpFrameData->mpFocusWin = this;
                if ( !mpWindowImpl->mpFrameData->mbInSysObjFocusHdl )
                    mpWindowImpl->mpSysObj->GrabFocus();
            }

            if ( pSVData->maWinData.mpFocusWin == this )
            {
                if ( mpWindowImpl->mpCursor )
                    mpWindowImpl->mpCursor->ImplShow();
                mpWindowImpl->mbInFocusHdl = true;
                mpWindowImpl->mnGetFocusFlags = nFlags;
                // if we're changing focus due to closing a popup floating window
                // notify the new focus window so it can restore the inner focus
                // eg, toolboxes can select their recent active item
                if( pOldFocusWindow &&
                    ! aOldFocusDel.IsDead() &&
                    ( pOldFocusWindow->GetDialogControlFlags() & WINDOW_DLGCTRL_FLOATWIN_POPUPMODEEND_CANCEL ) )
                    mpWindowImpl->mnGetFocusFlags |= GETFOCUS_FLOATWIN_POPUPMODEEND_CANCEL;
                NotifyEvent aNEvt( EVENT_GETFOCUS, this );
                if ( !ImplCallPreNotify( aNEvt ) && !aDogTag.IsDead() )
                    GetFocus();
                if( !aDogTag.IsDead() )
                    ImplCallActivateListeners( (pOldFocusWindow && ! aOldFocusDel.IsDead()) ? pOldFocusWindow : NULL );
                if( !aDogTag.IsDead() )
                {
                    mpWindowImpl->mnGetFocusFlags = 0;
                    mpWindowImpl->mbInFocusHdl = false;
                }
            }
        }

        GetpApp()->FocusChanged();
        ImplNewInputContext();
    }
}

void Window::ImplGrabFocusToDocument( sal_uInt16 nFlags )
{
    Window *pWin = this;
    while( pWin )
    {
        if( !pWin->GetParent() )
        {
            pWin->ImplGetFrameWindow()->GetWindow( WINDOW_CLIENT )->ImplGrabFocus(nFlags);
            return;
        }
        pWin = pWin->GetParent();
    }
}

void Window::ImplNewInputContext()
{
    ImplSVData* pSVData = ImplGetSVData();
    Window*     pFocusWin = pSVData->maWinData.mpFocusWin;
    if ( !pFocusWin )
        return;

    // Is InputContext changed?
    const InputContext& rInputContext = pFocusWin->GetInputContext();
    if ( rInputContext == pFocusWin->mpWindowImpl->mpFrameData->maOldInputContext )
        return;

    pFocusWin->mpWindowImpl->mpFrameData->maOldInputContext = rInputContext;

    SalInputContext         aNewContext;
    const Font&             rFont = rInputContext.GetFont();
    const OUString&         rFontName = rFont.GetName();
    ImplFontEntry*          pFontEntry = NULL;
    aNewContext.mpFont = NULL;
    if (!rFontName.isEmpty())
    {
        OutputDevice *pFocusWinOutDev = pFocusWin->GetOutDev();
        Size aSize = pFocusWinOutDev->ImplLogicToDevicePixel( rFont.GetSize() );
        if ( !aSize.Height() )
        {
            // only set default sizes if the font height in logical
            // coordinates equals 0
            if ( rFont.GetSize().Height() )
                aSize.Height() = 1;
            else
                aSize.Height() = (12*pFocusWin->mnDPIY)/72;
        }
        pFontEntry = pFocusWin->mpFontCache->GetFontEntry( pFocusWin->mpFontCollection,
                         rFont, aSize, static_cast<float>(aSize.Height()) );
        if ( pFontEntry )
            aNewContext.mpFont = &pFontEntry->maFontSelData;
    }
    aNewContext.meLanguage  = rFont.GetLanguage();
    aNewContext.mnOptions   = rInputContext.GetOptions();
    pFocusWin->ImplGetFrame()->SetInputContext( &aNewContext );

    if ( pFontEntry )
        pFocusWin->mpFontCache->Release( pFontEntry );
}

#if OSL_DEBUG_LEVEL > 0
namespace
{
    OString lcl_createWindowInfo(const Window& i_rWindow)
    {
        // skip border windows, they don't carry information which helps diagnosing the problem
        const Window* pWindow( &i_rWindow );
        while ( pWindow && ( pWindow->GetType() == WINDOW_BORDERWINDOW ) )
            pWindow = pWindow->GetWindow( WINDOW_FIRSTCHILD );
        if ( !pWindow )
            pWindow = &i_rWindow;

        OStringBuffer aErrorString;
        aErrorString.append(' ');
        aErrorString.append(typeid( *pWindow ).name());
        aErrorString.append(" (");
        aErrorString.append(OUStringToOString(pWindow->GetText(), RTL_TEXTENCODING_UTF8));
        aErrorString.append(")");
        return aErrorString.makeStringAndClear();
    }
}
#endif

Window::~Window()
{
    vcl::LazyDeletor<Window>::Undelete( this );

    DBG_ASSERT( !mpWindowImpl->mbInDtor, "~Window - already in DTOR!" );

    // remove Key and Mouse events issued by Application::PostKey/MouseEvent
    Application::RemoveMouseAndKeyEvents( this );

    // Dispose of the canvas implementation (which, currently, has an
    // own wrapper window as a child to this one.
    uno::Reference< rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );
    if( xCanvas.is() )
    {
        uno::Reference < lang::XComponent > xCanvasComponent( xCanvas,
                                                              uno::UNO_QUERY );
        if( xCanvasComponent.is() )
            xCanvasComponent->dispose();
    }

    mpWindowImpl->mbInDtor = true;

    ImplCallEventListeners( VCLEVENT_OBJECT_DYING );

    // do not send child events for frames that were registered as native frames
    if( !ImplIsAccessibleNativeFrame() && mpWindowImpl->mbReallyVisible )
        if ( ImplIsAccessibleCandidate() && GetAccessibleParentWindow() )
            GetAccessibleParentWindow()->ImplCallEventListeners( VCLEVENT_WINDOW_CHILDDESTROYED, this );

    // remove associated data structures from dockingmanager
    ImplGetDockingManager()->RemoveWindow( this );

    // remove ownerdraw decorated windows from list in the top-most frame window
    if( (GetStyle() & WB_OWNERDRAWDECORATION) && mpWindowImpl->mbFrame )
    {
        ::std::vector< Window* >& rList = ImplGetOwnerDrawList();
        ::std::vector< Window* >::iterator p;
        p = ::std::find( rList.begin(), rList.end(), this );
        if( p != rList.end() )
            rList.erase( p );
    }

    // shutdown drag and drop
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XComponent > xDnDComponent( mpWindowImpl->mxDNDListenerContainer, ::com::sun::star::uno::UNO_QUERY );

    if( xDnDComponent.is() )
        xDnDComponent->dispose();

    if( mpWindowImpl->mbFrame && mpWindowImpl->mpFrameData )
    {
        try
        {
            // deregister drop target listener
            if( mpWindowImpl->mpFrameData->mxDropTargetListener.is() )
            {
                uno::Reference< XDragGestureRecognizer > xDragGestureRecognizer =
                    uno::Reference< XDragGestureRecognizer > (mpWindowImpl->mpFrameData->mxDragSource, UNO_QUERY);
                if( xDragGestureRecognizer.is() )
                {
                    xDragGestureRecognizer->removeDragGestureListener(
                        uno::Reference< XDragGestureListener > (mpWindowImpl->mpFrameData->mxDropTargetListener, UNO_QUERY));
                }

                mpWindowImpl->mpFrameData->mxDropTarget->removeDropTargetListener( mpWindowImpl->mpFrameData->mxDropTargetListener );
                mpWindowImpl->mpFrameData->mxDropTargetListener.clear();
            }

            // shutdown drag and drop for this frame window
            uno::Reference< XComponent > xComponent( mpWindowImpl->mpFrameData->mxDropTarget, UNO_QUERY );

            // DNDEventDispatcher does not hold a reference of the DropTarget,
            // so it's ok if it does not support XComponent
            if( xComponent.is() )
                xComponent->dispose();
        }
        catch (const Exception&)
        {
            // can be safely ignored here.
        }
    }

    UnoWrapperBase* pWrapper = Application::GetUnoWrapper( false );
    if ( pWrapper )
        pWrapper->WindowDestroyed( this );

    // MT: Must be called after WindowDestroyed!
    // Otherwise, if the accessible is a VCLXWindow, it will try to destroy this window again!
    // But accessibility implementations from applications need this dispose.
    if ( mpWindowImpl->mxAccessible.is() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent> xC( mpWindowImpl->mxAccessible, ::com::sun::star::uno::UNO_QUERY );
        if ( xC.is() )
            xC->dispose();
    }

    ImplSVData* pSVData = ImplGetSVData();

    if ( pSVData->maHelpData.mpHelpWin && (pSVData->maHelpData.mpHelpWin->GetParent() == this) )
        ImplDestroyHelpWindow( true );

    DBG_ASSERT( pSVData->maWinData.mpTrackWin != this,
                "Window::~Window(): Window is in TrackingMode" );
    DBG_ASSERT( pSVData->maWinData.mpCaptureWin != this,
                "Window::~Window(): Window has the mouse captured" );
    // #103442# DefModalDialogParent is now determined on-the-fly, so this pointer is unimportant now
    //DBG_ASSERT( pSVData->maWinData.mpDefDialogParent != this,
    //            "Window::~Window(): Window is DefModalDialogParent" );

    // due to old compatibility
    if ( pSVData->maWinData.mpTrackWin == this )
        EndTracking();
    if ( pSVData->maWinData.mpCaptureWin == this )
        ReleaseMouse();
    if ( pSVData->maWinData.mpDefDialogParent == this )
        pSVData->maWinData.mpDefDialogParent = NULL;

#if OSL_DEBUG_LEVEL > 0
    if ( true ) // always perform these tests in non-pro versions
    {
        OStringBuffer aErrorStr;
        bool        bError = false;
        Window*     pTempWin;
        if (mpWindowImpl->mpFrameData != 0)
        {
            pTempWin = mpWindowImpl->mpFrameData->mpFirstOverlap;
            while ( pTempWin )
            {
                if ( ImplIsRealParentPath( pTempWin ) )
                {
                    bError = true;
                    aErrorStr.append(lcl_createWindowInfo(*pTempWin));
                }
                pTempWin = pTempWin->mpWindowImpl->mpNextOverlap;
            }
            if ( bError )
            {
                OStringBuffer aTempStr;
                aTempStr.append("Window (");
                aTempStr.append(OUStringToOString(GetText(),
                                                       RTL_TEXTENCODING_UTF8));
                aTempStr.append(") with live SystemWindows destroyed: ");
                aTempStr.append(aErrorStr.toString());
                OSL_FAIL(aTempStr.getStr());
                // abort in non-pro version, this must be fixed!
                GetpApp()->Abort(OStringToOUString(
                                     aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));
            }
        }

        bError = false;
        pTempWin = pSVData->maWinData.mpFirstFrame;
        while ( pTempWin )
        {
            if ( ImplIsRealParentPath( pTempWin ) )
            {
                bError = true;
                aErrorStr.append(lcl_createWindowInfo(*pTempWin));
            }
            pTempWin = pTempWin->mpWindowImpl->mpFrameData->mpNextFrame;
        }
        if ( bError )
        {
            OStringBuffer aTempStr( "Window (" );
            aTempStr.append(OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") with live SystemWindows destroyed: ");
            aTempStr.append(aErrorStr.toString());
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in non-pro version, this must be fixed!
        }

        if ( mpWindowImpl->mpFirstChild )
        {
            OStringBuffer aTempStr("Window (");
            aTempStr.append(OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") with live children destroyed: ");
            pTempWin = mpWindowImpl->mpFirstChild;
            while ( pTempWin )
            {
                aTempStr.append(lcl_createWindowInfo(*pTempWin));
                pTempWin = pTempWin->mpWindowImpl->mpNext;
            }
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in non-pro version, this must be fixed!
        }

        if ( mpWindowImpl->mpFirstOverlap )
        {
            OStringBuffer aTempStr("Window (");
            aTempStr.append(OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") with live SystemWindows destroyed: ");
            pTempWin = mpWindowImpl->mpFirstOverlap;
            while ( pTempWin )
            {
                aTempStr.append(lcl_createWindowInfo(*pTempWin));
                pTempWin = pTempWin->mpWindowImpl->mpNext;
            }
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in non-pro version, this must be fixed!
        }

        Window* pMyParent = this;
        SystemWindow* pMySysWin = NULL;

        while ( pMyParent )
        {
            if ( pMyParent->IsSystemWindow() )
                pMySysWin = (SystemWindow*)pMyParent;
            pMyParent = pMyParent->GetParent();
        }
        if ( pMySysWin && pMySysWin->ImplIsInTaskPaneList( this ) )
        {
            OStringBuffer aTempStr("Window (");
            aTempStr.append(OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") still in TaskPanelList!");
            OSL_FAIL( aTempStr.getStr() );
            GetpApp()->Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8));   // abort in non-pro version, this must be fixed!
        }
    }
#endif

    if( mpWindowImpl->mbIsInTaskPaneList )
    {
        Window* pMyParent = this;
        SystemWindow* pMySysWin = NULL;

        while ( pMyParent )
        {
            if ( pMyParent->IsSystemWindow() )
                pMySysWin = (SystemWindow*)pMyParent;
            pMyParent = pMyParent->GetParent();
        }
        if ( pMySysWin && pMySysWin->ImplIsInTaskPaneList( this ) )
        {
            pMySysWin->GetTaskPaneList()->RemoveWindow( this );
        }
        else
        {
            OStringBuffer aTempStr("Window (");
            aTempStr.append(OUStringToOString(GetText(), RTL_TEXTENCODING_UTF8));
            aTempStr.append(") not found in TaskPanelList!");
            OSL_FAIL( aTempStr.getStr() );
        }
    }

    // remove from size-group if necessary
    remove_from_all_size_groups();

    // clear mnemonic labels
    std::vector<FixedText*> aMnemonicLabels(list_mnemonic_labels());
    for (std::vector<FixedText*>::iterator aI = aMnemonicLabels.begin();
        aI != aMnemonicLabels.end(); ++aI)
    {
        remove_mnemonic_label(*aI);
    }

    // hide window in order to trigger the Paint-Handling
    Hide();

    // announce the window is to be destroyed
    {
        NotifyEvent aNEvt( EVENT_DESTROY, this );
        Notify( aNEvt );
    }

    // EndExtTextInputMode
    if ( pSVData->maWinData.mpExtTextInputWin == this )
    {
        EndExtTextInput( EXTTEXTINPUT_END_COMPLETE );
        if ( pSVData->maWinData.mpExtTextInputWin == this )
            pSVData->maWinData.mpExtTextInputWin = NULL;
    }

    // check if the focus window is our child
    bool bHasFocussedChild = false;
    if( pSVData->maWinData.mpFocusWin && ImplIsRealParentPath( pSVData->maWinData.mpFocusWin ) )
    {
        // #122232#, this must not happen and is an application bug ! but we try some cleanup to hopefully avoid crashes, see below
        bHasFocussedChild = true;
#if OSL_DEBUG_LEVEL > 0
        OStringBuffer aTempStr("Window (");
        aTempStr.append(OUStringToOString(GetText(),
            RTL_TEXTENCODING_UTF8)).
                append(") with focussed child window destroyed ! THIS WILL LEAD TO CRASHES AND MUST BE FIXED !");
        OSL_FAIL( aTempStr.getStr() );
        GetpApp()->Abort(OStringToOUString(aTempStr.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ));   // abort in non-pro version, this must be fixed!
#endif
    }

    // if we get focus pass focus to another window
    Window* pOverlapWindow = ImplGetFirstOverlapWindow();
    if ( pSVData->maWinData.mpFocusWin == this
        || bHasFocussedChild )  // #122232#, see above, try some cleanup
    {
        if ( mpWindowImpl->mbFrame )
        {
            pSVData->maWinData.mpFocusWin = NULL;
            pOverlapWindow->mpWindowImpl->mpLastFocusWindow = NULL;
            GetpApp()->FocusChanged();
        }
        else
        {
            Window* pParent = GetParent();
            Window* pBorderWindow = mpWindowImpl->mpBorderWindow;
        // when windows overlap, give focus to the parent
        // of the next FrameWindow
            if ( pBorderWindow )
            {
                if ( pBorderWindow->ImplIsOverlapWindow() )
                    pParent = pBorderWindow->mpWindowImpl->mpOverlapWindow;
            }
            else if ( ImplIsOverlapWindow() )
                pParent = mpWindowImpl->mpOverlapWindow;

            if ( pParent && pParent->IsEnabled() && pParent->IsInputEnabled() && ! pParent->IsInModalMode() )
                pParent->GrabFocus();
            else
                mpWindowImpl->mpFrameWindow->GrabFocus();

            // If the focus was set back to 'this' set it to nothing
            if ( pSVData->maWinData.mpFocusWin == this )
            {
                pSVData->maWinData.mpFocusWin = NULL;
                pOverlapWindow->mpWindowImpl->mpLastFocusWindow = NULL;
                GetpApp()->FocusChanged();
            }
        }
    }

    if ( pOverlapWindow != 0 &&
         pOverlapWindow->mpWindowImpl->mpLastFocusWindow == this )
        pOverlapWindow->mpWindowImpl->mpLastFocusWindow = NULL;

    // reset hint for DefModalDialogParent
    if( pSVData->maWinData.mpActiveApplicationFrame == this )
        pSVData->maWinData.mpActiveApplicationFrame = NULL;

    // reset marked windows
    if ( mpWindowImpl->mpFrameData != 0 )
    {
        if ( mpWindowImpl->mpFrameData->mpFocusWin == this )
            mpWindowImpl->mpFrameData->mpFocusWin = NULL;
        if ( mpWindowImpl->mpFrameData->mpMouseMoveWin == this )
            mpWindowImpl->mpFrameData->mpMouseMoveWin = NULL;
        if ( mpWindowImpl->mpFrameData->mpMouseDownWin == this )
            mpWindowImpl->mpFrameData->mpMouseDownWin = NULL;
    }

    // reset Deactivate-Window
    if ( pSVData->maWinData.mpLastDeacWin == this )
        pSVData->maWinData.mpLastDeacWin = NULL;

    if ( mpWindowImpl->mbFrame && mpWindowImpl->mpFrameData )
    {
        if ( mpWindowImpl->mpFrameData->mnFocusId )
            Application::RemoveUserEvent( mpWindowImpl->mpFrameData->mnFocusId );
        if ( mpWindowImpl->mpFrameData->mnMouseMoveId )
            Application::RemoveUserEvent( mpWindowImpl->mpFrameData->mnMouseMoveId );
    }

    // release SalGraphics
    OutputDevice *pOutDev = GetOutDev();
    pOutDev->ReleaseGraphics();

    // notify ImplDelData subscribers of this window about the window deletion
    ImplDelData* pDelData = mpWindowImpl->mpFirstDel;
    while ( pDelData )
    {
        pDelData->mbDel = true;
        pDelData->mpWindow = NULL;  // #112873# pDel is not associated with a Window anymore
        pDelData = pDelData->mpNext;
    }

    // remove window from the lists
    ImplRemoveWindow( true );

    // de-register as "top window child" at our parent, if necessary
    if ( mpWindowImpl->mbFrame )
    {
        bool bIsTopWindow = mpWindowImpl->mpWinData && ( mpWindowImpl->mpWinData->mnIsTopWindow == 1 );
        if ( mpWindowImpl->mpRealParent && bIsTopWindow )
        {
            ImplWinData* pParentWinData = mpWindowImpl->mpRealParent->ImplGetWinData();

            ::std::list< Window* >::iterator myPos = ::std::find( pParentWinData->maTopWindowChildren.begin(),
                pParentWinData->maTopWindowChildren.end(), this );
            DBG_ASSERT( myPos != pParentWinData->maTopWindowChildren.end(), "Window::~Window: inconsistency in top window chain!" );
            if ( myPos != pParentWinData->maTopWindowChildren.end() )
                pParentWinData->maTopWindowChildren.erase( myPos );
        }
    }

    // cleanup Extra Window Data, TODO: add and use ImplWinData destructor
    if ( mpWindowImpl->mpWinData )
    {
        if ( mpWindowImpl->mpWinData->mpExtOldText )
            delete mpWindowImpl->mpWinData->mpExtOldText;
        if ( mpWindowImpl->mpWinData->mpExtOldAttrAry )
            delete mpWindowImpl->mpWinData->mpExtOldAttrAry;
        if ( mpWindowImpl->mpWinData->mpCursorRect )
            delete mpWindowImpl->mpWinData->mpCursorRect;
        if ( mpWindowImpl->mpWinData->mpCompositionCharRects)
            delete[] mpWindowImpl->mpWinData->mpCompositionCharRects;
        if ( mpWindowImpl->mpWinData->mpFocusRect )
            delete mpWindowImpl->mpWinData->mpFocusRect;
        if ( mpWindowImpl->mpWinData->mpTrackRect )
            delete mpWindowImpl->mpWinData->mpTrackRect;

        delete mpWindowImpl->mpWinData;
    }

    // cleanup overlap related window data
    if ( mpWindowImpl->mpOverlapData )
        delete mpWindowImpl->mpOverlapData;

    // remove BorderWindow or Frame window data
    if ( mpWindowImpl->mpBorderWindow )
        delete mpWindowImpl->mpBorderWindow;
    else if ( mpWindowImpl->mbFrame )
    {
        if ( pSVData->maWinData.mpFirstFrame == this )
            pSVData->maWinData.mpFirstFrame = mpWindowImpl->mpFrameData->mpNextFrame;
        else
        {
            Window* pSysWin = pSVData->maWinData.mpFirstFrame;
            while ( pSysWin->mpWindowImpl->mpFrameData->mpNextFrame != this )
                pSysWin = pSysWin->mpWindowImpl->mpFrameData->mpNextFrame;
            pSysWin->mpWindowImpl->mpFrameData->mpNextFrame = mpWindowImpl->mpFrameData->mpNextFrame;
        }
        mpWindowImpl->mpFrame->SetCallback( NULL, NULL );
        pSVData->mpDefInst->DestroyFrame( mpWindowImpl->mpFrame );
        delete mpWindowImpl->mpFrameData;
    }

    // should be the last statements
    delete mpWindowImpl; mpWindowImpl = NULL;
}

void Window::doLazyDelete()
{
    SystemWindow* pSysWin = dynamic_cast<SystemWindow*>(this);
    DockingWindow* pDockWin = dynamic_cast<DockingWindow*>(this);
    if( pSysWin || ( pDockWin && pDockWin->IsFloatingMode() ) )
    {
        Show( false );
        SetParent( ImplGetDefaultWindow() );
    }
    vcl::LazyDeletor<Window>::Delete( this );
}

sal_uInt16 Window::GetIndicatorState() const
{
    return mpWindowImpl->mpFrame->GetIndicatorState().mnState;
}

void Window::SimulateKeyPress( sal_uInt16 nKeyCode ) const
{
    mpWindowImpl->mpFrame->SimulateKeyPress(nKeyCode);
}

void Window::MouseMove( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( EVENT_MOUSEMOVE, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseMove = true;
}

void Window::MouseButtonDown( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( EVENT_MOUSEBUTTONDOWN, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseButtonDown = true;
}

void Window::MouseButtonUp( const MouseEvent& rMEvt )
{
    NotifyEvent aNEvt( EVENT_MOUSEBUTTONUP, this, &rMEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbMouseButtonUp = true;
}

void Window::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode cod = rKEvt.GetKeyCode ();
    bool accel = ImplGetSVData()->maNWFData.mbEnableAccel;
    bool autoacc = ImplGetSVData()->maNWFData.mbAutoAccel;

    // do not respond to accelerators unless Alt is held */
    if (cod.GetCode () >= 0x200 && cod.GetCode () <= 0x219)
    {
        if (!accel) return;
        if (autoacc && cod.GetModifier () != 0x4000) return;
    }

    NotifyEvent aNEvt( EVENT_KEYINPUT, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbKeyInput = true;
}

void Window::KeyUp( const KeyEvent& rKEvt )
{
    NotifyEvent aNEvt( EVENT_KEYUP, this, &rKEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbKeyUp = true;
}

void Window::PrePaint()
{
}

void Window::Paint( const Rectangle& rRect )
{
    ImplCallEventListeners( VCLEVENT_WINDOW_PAINT, (void*)&rRect );
}

void Window::PostPaint()
{
}

void Window::Draw( OutputDevice*, const Point&, const Size&, sal_uLong )
{
}

void Window::Move() {}

void Window::Resize() {}

void Window::Activate() {}

void Window::Deactivate() {}

void Window::GetFocus()
{
    if ( HasFocus() && mpWindowImpl->mpLastFocusWindow && !(mpWindowImpl->mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) )
    {
        ImplDelData aDogtag( this );
        mpWindowImpl->mpLastFocusWindow->GrabFocus();
        if( aDogtag.IsDead() )
            return;
    }

    NotifyEvent aNEvt( EVENT_GETFOCUS, this );
    Notify( aNEvt );
}

void Window::LoseFocus()
{
    NotifyEvent aNEvt( EVENT_LOSEFOCUS, this );
    Notify( aNEvt );
}

void Window::RequestHelp( const HelpEvent& rHEvt )
{
    // if Balloon-Help is requested, show the balloon
    // with help text set
    if ( rHEvt.GetMode() & HELPMODE_BALLOON )
    {
        OUString rStr = GetHelpText();
        if ( rStr.isEmpty() )
            rStr = GetQuickHelpText();
        if ( rStr.isEmpty() && ImplGetParent() && !ImplIsOverlapWindow() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
            Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), rStr );
    }
    else if ( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        const OUString& rStr = GetQuickHelpText();
        if ( rStr.isEmpty() && ImplGetParent() && !ImplIsOverlapWindow() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            Point aPos = GetPosPixel();
            if ( ImplGetParent() && !ImplIsOverlapWindow() )
                aPos = ImplGetParent()->OutputToScreenPixel( aPos );
            Rectangle   aRect( aPos, GetSizePixel() );
            OUString      aHelpText;
            if ( !rStr.isEmpty() )
                aHelpText = GetHelpText();
            Help::ShowQuickHelp( this, aRect, rStr, aHelpText, QUICKHELP_CTRLTEXT );
        }
    }
    else
    {
        OUString aStrHelpId( OStringToOUString( GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
        if ( aStrHelpId.isEmpty() && ImplGetParent() )
            ImplGetParent()->RequestHelp( rHEvt );
        else
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                if( !aStrHelpId.isEmpty() )
                    pHelp->Start( aStrHelpId, this );
                else
                    pHelp->Start( OUString( OOO_HELP_INDEX  ), this );
            }
        }
    }
}

void Window::Command( const CommandEvent& rCEvt )
{
    ImplCallEventListeners( VCLEVENT_WINDOW_COMMAND, (void*)&rCEvt );

    NotifyEvent aNEvt( EVENT_COMMAND, this, &rCEvt );
    if ( !Notify( aNEvt ) )
        mpWindowImpl->mbCommand = true;
}

void Window::Tracking( const TrackingEvent& rTEvt )
{

    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper )
        pWrapper->Tracking( rTEvt );
}

void Window::StateChanged( StateChangedType eType )
{
    switch (eType)
    {
        //stuff that doesn't invalidate the layout
        case STATE_CHANGE_CONTROLFOREGROUND:
        case STATE_CHANGE_CONTROLBACKGROUND:
        case STATE_CHANGE_TRANSPARENT:
        case STATE_CHANGE_UPDATEMODE:
        case STATE_CHANGE_READONLY:
        case STATE_CHANGE_ENABLE:
        case STATE_CHANGE_STATE:
        case STATE_CHANGE_DATA:
        case STATE_CHANGE_INITSHOW:
        case STATE_CHANGE_CONTROL_FOCUS:
            break;
        //stuff that does invalidate the layout
        default:
            queue_resize();
            break;
    }
}

void Window::DataChanged( const DataChangedEvent& )
{
}

void Window::ImplNotifyKeyMouseCommandEventListeners( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_COMMAND )
    {
        const CommandEvent* pCEvt = rNEvt.GetCommandEvent();
        if ( pCEvt->GetCommand() != COMMAND_CONTEXTMENU )
            // non context menu events are not to be notified up the chain
            // so we return immediately
            return;

        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                // not interested in: The event listeners are already called in ::Command,
                // and calling them here a second time doesn't make sense
                ;
            else
            {
                CommandEvent aCommandEvent = ImplTranslateCommandEvent( *pCEvt, rNEvt.GetWindow(), this );
                ImplCallEventListeners( VCLEVENT_WINDOW_COMMAND, &aCommandEvent );
            }
        }
    }

    // #82968# notify event listeners for mouse and key events separately and
    // not in PreNotify ( as for focus listeners )
    // this allows for processing those events internally first and pass it to
    // the toolkit later

    ImplDelData aDelData;
    ImplAddDel( &aDelData );

    if( rNEvt.GetType() == EVENT_MOUSEMOVE )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEMOVE, (void*)rNEvt.GetMouseEvent() );
            else
            {
                MouseEvent aMouseEvent = ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this );
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEMOVE, &aMouseEvent );
            }
        }
    }
    else if( rNEvt.GetType() == EVENT_MOUSEBUTTONUP )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONUP, (void*)rNEvt.GetMouseEvent() );
            else
            {
                MouseEvent aMouseEvent = ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this );
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONUP, &aMouseEvent );
            }
        }
    }
    else if( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
        {
            if ( rNEvt.GetWindow() == this )
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, (void*)rNEvt.GetMouseEvent() );
            else
            {
                MouseEvent aMouseEvent = ImplTranslateMouseEvent( *rNEvt.GetMouseEvent(), rNEvt.GetWindow(), this );
                ImplCallEventListeners( VCLEVENT_WINDOW_MOUSEBUTTONDOWN, &aMouseEvent );
            }
        }
    }
    else if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
            ImplCallEventListeners( VCLEVENT_WINDOW_KEYINPUT, (void*)rNEvt.GetKeyEvent() );
    }
    else if( rNEvt.GetType() == EVENT_KEYUP )
    {
        if ( mpWindowImpl->mbCompoundControl || ( rNEvt.GetWindow() == this ) )
            ImplCallEventListeners( VCLEVENT_WINDOW_KEYUP, (void*)rNEvt.GetKeyEvent() );
    }

    if ( aDelData.IsDead() )
        return;
    ImplRemoveDel( &aDelData );

    // #106721# check if we're part of a compound control and notify
    Window *pParent = ImplGetParent();
    while( pParent )
    {
        if( pParent->IsCompoundControl() )
        {
            pParent->ImplNotifyKeyMouseCommandEventListeners( rNEvt );
            break;
        }
        pParent = pParent->ImplGetParent();
    }
}

bool Window::PreNotify( NotifyEvent& rNEvt )
{
    bool bDone = false;
    if ( mpWindowImpl->mpParent && !ImplIsOverlapWindow() )
        bDone = mpWindowImpl->mpParent->PreNotify( rNEvt );

    if ( !bDone )
    {
        if( rNEvt.GetType() == EVENT_GETFOCUS )
        {
            bool bCompoundFocusChanged = false;
            if ( mpWindowImpl->mbCompoundControl && !mpWindowImpl->mbCompoundControlHasFocus && HasChildPathFocus() )
            {
                mpWindowImpl->mbCompoundControlHasFocus = true;
                bCompoundFocusChanged = true;
            }

            if ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) )
                ImplCallEventListeners( VCLEVENT_WINDOW_GETFOCUS );
        }
        else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
        {
            bool bCompoundFocusChanged = false;
            if ( mpWindowImpl->mbCompoundControl && mpWindowImpl->mbCompoundControlHasFocus && !HasChildPathFocus() )
            {
                mpWindowImpl->mbCompoundControlHasFocus = false ;
                bCompoundFocusChanged = true;
            }

            if ( bCompoundFocusChanged || ( rNEvt.GetWindow() == this ) )
                ImplCallEventListeners( VCLEVENT_WINDOW_LOSEFOCUS );
        }

        // #82968# mouse and key events will be notified after processing ( in ImplNotifyKeyMouseCommandEventListeners() )!
        //    see also ImplHandleMouseEvent(), ImplHandleKey()

    }

    return bDone;
}

bool Window::Notify( NotifyEvent& rNEvt )
{
    bool nRet = false;

    // check for docking window
    // but do nothing if window is docked and locked
    ImplDockingWindowWrapper *pWrapper = ImplGetDockingManager()->GetDockingWindowWrapper( this );
    if( pWrapper && !( !pWrapper->IsFloatingMode() && pWrapper->IsLocked() ) )
    {
        if ( rNEvt.GetType() == EVENT_MOUSEBUTTONDOWN )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            bool bHit = pWrapper->GetDragArea().IsInside( pMEvt->GetPosPixel() );
            if ( pMEvt->IsLeft() )
            {
                if ( pMEvt->IsMod1() && (pMEvt->GetClicks() == 2) )
                {
                    // ctrl double click toggles floating mode
                    pWrapper->SetFloatingMode( !pWrapper->IsFloatingMode() );
                    return true;
                }
                else if ( pMEvt->GetClicks() == 1 && bHit)
                {
                    // allow start docking during mouse move
                    pWrapper->ImplEnableStartDocking();
                    return true;
                }
            }
        }
        else if ( rNEvt.GetType() == EVENT_MOUSEMOVE )
        {
            const MouseEvent* pMEvt = rNEvt.GetMouseEvent();
            bool bHit = pWrapper->GetDragArea().IsInside( pMEvt->GetPosPixel() );
            if ( pMEvt->IsLeft() )
            {
                // check if a single click initiated this sequence ( ImplStartDockingEnabled() )
                // check if window is docked and
                if( pWrapper->ImplStartDockingEnabled() && !pWrapper->IsFloatingMode() &&
                    !pWrapper->IsDocking() && bHit )
                {
                    Point   aPos = pMEvt->GetPosPixel();
                    Window* pWindow = rNEvt.GetWindow();
                    if ( pWindow != this )
                    {
                        aPos = pWindow->OutputToScreenPixel( aPos );
                        aPos = ScreenToOutputPixel( aPos );
                    }
                    pWrapper->ImplStartDocking( aPos );
                }
                return true;
            }
        }
        else if( rNEvt.GetType() == EVENT_KEYINPUT )
        {
            const KeyCode& rKey = rNEvt.GetKeyEvent()->GetKeyCode();
            if( rKey.GetCode() == KEY_F10 && rKey.GetModifier() &&
                rKey.IsShift() && rKey.IsMod1() )
            {
                pWrapper->SetFloatingMode( !pWrapper->IsFloatingMode() );
                /* At this point the floating toolbar frame does not have the
                 * input focus since these frames don't get the focus per default
                 * To enable keyboard handling of this toolbar set the input focus
                 * to the frame. This needs to be done with ToTop since GrabFocus
                 * would not notice any change since "this" already has the focus.
                 */
                if( pWrapper->IsFloatingMode() )
                    ToTop( TOTOP_GRABFOCUSONLY );
                return true;
            }
        }
    }

    // manage the dialogs
    if ( (GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) == WB_DIALOGCONTROL )
    {
        // if the parent also has dialog control activated, the parent takes over control
        if ( (rNEvt.GetType() == EVENT_KEYINPUT) || (rNEvt.GetType() == EVENT_KEYUP) )
        {
            if ( ImplIsOverlapWindow() ||
                 ((getNonLayoutRealParent(this)->GetStyle() & (WB_DIALOGCONTROL | WB_NODIALOGCONTROL)) != WB_DIALOGCONTROL) )
            {
                nRet = ImplDlgCtrl( *rNEvt.GetKeyEvent(), rNEvt.GetType() == EVENT_KEYINPUT );
            }
        }
        else if ( (rNEvt.GetType() == EVENT_GETFOCUS) || (rNEvt.GetType() == EVENT_LOSEFOCUS) )
        {
            ImplDlgCtrlFocusChanged( rNEvt.GetWindow(), rNEvt.GetType() == EVENT_GETFOCUS );
            if ( (rNEvt.GetWindow() == this) && (rNEvt.GetType() == EVENT_GETFOCUS) &&
                 !(GetStyle() & WB_TABSTOP) && !(mpWindowImpl->mnDlgCtrlFlags & WINDOW_DLGCTRL_WANTFOCUS) )
            {
                sal_uInt16 n = 0;
                Window* pFirstChild = ImplGetDlgWindow( n, DLGWINDOW_FIRST );
                if ( pFirstChild )
                    pFirstChild->ImplControlFocus();
            }
        }
    }

    if ( !nRet )
    {
        if ( mpWindowImpl->mpParent && !ImplIsOverlapWindow() )
            nRet = mpWindowImpl->mpParent->Notify( rNEvt );
    }

    return nRet;
}

void Window::ImplCallEventListeners( sal_uLong nEvent, void* pData )
{
    // The implementation was moved to CallEventListeners(),
    // because derived classes in svtools must be able to
    // call the event listeners and ImplCallEventListeners()
    // is not exported.
    // TODO: replace ImplCallEventListeners() by CallEventListeners() in vcl

    CallEventListeners( nEvent, pData );
}

void Window::CallEventListeners( sal_uLong nEvent, void* pData )
{
    VclWindowEvent aEvent( this, nEvent, pData );

    ImplDelData aDelData;
    ImplAddDel( &aDelData );

    ImplGetSVData()->mpApp->ImplCallEventListeners( &aEvent );

    if ( aDelData.IsDead() )
        return;

    mpWindowImpl->maEventListeners.Call( &aEvent );

    if ( aDelData.IsDead() )
        return;

    ImplRemoveDel( &aDelData );

    Window* pWindow = this;
    while ( pWindow )
    {
        pWindow->ImplAddDel( &aDelData );

        pWindow->mpWindowImpl->maChildEventListeners.Call( &aEvent );

        if ( aDelData.IsDead() )
            return;

        pWindow->ImplRemoveDel( &aDelData );

        pWindow = pWindow->GetParent();
    }
}

void Window::FireVclEvent( VclSimpleEvent* pEvent )
{
    ImplGetSVData()->mpApp->ImplCallEventListeners(pEvent);
}

void Window::AddEventListener( const Link& rEventListener )
{
    mpWindowImpl->maEventListeners.addListener( rEventListener );
}

void Window::RemoveEventListener( const Link& rEventListener )
{
    mpWindowImpl->maEventListeners.removeListener( rEventListener );
}

void Window::AddChildEventListener( const Link& rEventListener )
{
    mpWindowImpl->maChildEventListeners.addListener( rEventListener );
}

void Window::RemoveChildEventListener( const Link& rEventListener )
{
    mpWindowImpl->maChildEventListeners.removeListener( rEventListener );
}

ImplSVEvent * Window::PostUserEvent( const Link& rLink, void* pCaller )
{
    ImplSVEvent* pSVEvent = new ImplSVEvent;
    pSVEvent->mpData    = pCaller;
    pSVEvent->mpLink    = new Link( rLink );
    pSVEvent->mpWindow  = this;
    pSVEvent->mbCall    = true;
    ImplAddDel( &(pSVEvent->maDelData) );
    if ( !mpWindowImpl->mpFrame->PostEvent( pSVEvent ) )
    {
        ImplRemoveDel( &(pSVEvent->maDelData) );
        delete pSVEvent->mpLink;
        delete pSVEvent;
        pSVEvent = 0;
    }
    return pSVEvent;
}

void Window::RemoveUserEvent( ImplSVEvent * nUserEvent )
{
    DBG_ASSERT( nUserEvent->mpWindow == this,
                "Window::RemoveUserEvent(): Event doesn't send to this window or is already removed" );
    DBG_ASSERT( nUserEvent->mbCall,
                "Window::RemoveUserEvent(): Event is already removed" );

    if ( nUserEvent->mpWindow )
    {
        nUserEvent->mpWindow->ImplRemoveDel( &(nUserEvent->maDelData) );
        nUserEvent->mpWindow = NULL;
    }

    nUserEvent->mbCall = false;
}

bool Window::IsLocked( bool bChildren ) const
{
    if ( mpWindowImpl->mnLockCount != 0 )
        return true;

    if ( bChildren || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            if ( pChild->IsLocked( true ) )
                return true;
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }

    return false;
}

void Window::SetStyle( WinBits nStyle )
{

    if ( mpWindowImpl->mnStyle != nStyle )
    {
        mpWindowImpl->mnPrevStyle = mpWindowImpl->mnStyle;
        mpWindowImpl->mnStyle = nStyle;
        StateChanged( STATE_CHANGE_STYLE );
    }
}

void Window::SetExtendedStyle( WinBits nExtendedStyle )
{

    if ( mpWindowImpl->mnExtendedStyle != nExtendedStyle )
    {
        Window* pWindow = ImplGetBorderWindow();
        if( ! pWindow )
            pWindow = this;
        if( pWindow->mpWindowImpl->mbFrame )
        {
            SalExtStyle nExt = 0;
            if( (nExtendedStyle & WB_EXT_DOCUMENT) )
                nExt |= SAL_FRAME_EXT_STYLE_DOCUMENT;
            if( (nExtendedStyle & WB_EXT_DOCMODIFIED) )
                nExt |= SAL_FRAME_EXT_STYLE_DOCMODIFIED;

            pWindow->ImplGetFrame()->SetExtendedFrameStyle( nExt );
        }
        mpWindowImpl->mnPrevExtendedStyle = mpWindowImpl->mnExtendedStyle;
        mpWindowImpl->mnExtendedStyle = nExtendedStyle;
        StateChanged( STATE_CHANGE_EXTENDEDSTYLE );
    }
}

SystemWindow* Window::GetSystemWindow() const
{

    const Window* pWin = this;
    while ( pWin && !pWin->IsSystemWindow() )
        pWin  = pWin->GetParent();
    return (SystemWindow*)pWin;
}

void Window::SetBorderStyle( sal_uInt16 nBorderStyle )
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        if( nBorderStyle == WINDOW_BORDER_REMOVEBORDER &&
            ! mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame &&
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mpParent
            )
        {
            // this is a little awkward: some controls (e.g. svtools ProgressBar)
            // cannot avoid getting constructed with WB_BORDER but want to disable
            // borders in case of NWF drawing. So they need a method to remove their border window
            Window* pBorderWin = mpWindowImpl->mpBorderWindow;
            // remove us as border window's client
            pBorderWin->mpWindowImpl->mpClientWindow = NULL;
            mpWindowImpl->mpBorderWindow = NULL;
            mpWindowImpl->mpRealParent = pBorderWin->mpWindowImpl->mpParent;
            // reparent us above the border window
            SetParent( pBorderWin->mpWindowImpl->mpParent );
            // set us to the position and size of our previous border
            Point aBorderPos( pBorderWin->GetPosPixel() );
            Size aBorderSize( pBorderWin->GetSizePixel() );
            setPosSizePixel( aBorderPos.X(), aBorderPos.Y(), aBorderSize.Width(), aBorderSize.Height() );
            // release border window
            delete pBorderWin;

            // set new style bits
            SetStyle( GetStyle() & (~WB_BORDER) );
        }
        else
        {
            if ( mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
                 ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->SetBorderStyle( nBorderStyle );
            else
                mpWindowImpl->mpBorderWindow->SetBorderStyle( nBorderStyle );
        }
    }
}

sal_uInt16 Window::GetBorderStyle() const
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        if ( mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->GetBorderStyle();
        else
            return mpWindowImpl->mpBorderWindow->GetBorderStyle();
    }

    return 0;
}

long Window::CalcTitleWidth() const
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        if ( mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW )
            return ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->CalcTitleWidth();
        else
            return mpWindowImpl->mpBorderWindow->CalcTitleWidth();
    }
    else if ( mpWindowImpl->mbFrame && (mpWindowImpl->mnStyle & WB_MOVEABLE) )
    {
        // we guess the width for frame windows as we do not know the
        // border of external dialogs
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
        Font aFont = GetFont();
        ((Window*)this)->SetPointFont( rStyleSettings.GetTitleFont() );
        long nTitleWidth = GetTextWidth( GetText() );
        ((Window*)this)->SetFont( aFont );
        nTitleWidth += rStyleSettings.GetTitleHeight() * 3;
        nTitleWidth += rStyleSettings.GetBorderSize() * 2;
        nTitleWidth += 10;
        return nTitleWidth;
    }

    return 0;
}

void Window::SetMouseTransparent( bool bTransparent )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetMouseTransparent( bTransparent );

    if( mpWindowImpl->mpSysObj )
        mpWindowImpl->mpSysObj->SetMouseTransparent( bTransparent );

    mpWindowImpl->mbMouseTransparent = bTransparent;
}

void Window::SetPaintTransparent( bool bTransparent )
{

    // transparency is not useful for frames as the background would have to be provided by a different frame
    if( bTransparent && mpWindowImpl->mbFrame )
        return;

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetPaintTransparent( bTransparent );

    mpWindowImpl->mbPaintTransparent = bTransparent;
}

void Window::SetInputContext( const InputContext& rInputContext )
{

    mpWindowImpl->maInputContext = rInputContext;
    if ( !mpWindowImpl->mbInFocusHdl && HasFocus() )
        ImplNewInputContext();
}

void Window::EndExtTextInput( sal_uInt16 nFlags )
{

    if ( mpWindowImpl->mbExtTextInput )
        ImplGetFrame()->EndExtTextInput( nFlags );
}

void Window::SetCursorRect( const Rectangle* pRect, long nExtTextInputWidth )
{

    ImplWinData* pWinData = ImplGetWinData();
    if ( pWinData->mpCursorRect )
    {
        if ( pRect )
            *pWinData->mpCursorRect = *pRect;
        else
        {
            delete pWinData->mpCursorRect;
            pWinData->mpCursorRect = NULL;
        }
    }
    else
    {
        if ( pRect )
            pWinData->mpCursorRect = new Rectangle( *pRect );
    }

    pWinData->mnCursorExtWidth = nExtTextInputWidth;

}

const Rectangle* Window::GetCursorRect() const
{

    ImplWinData* pWinData = ImplGetWinData();
    return pWinData->mpCursorRect;
}

long Window::GetCursorExtTextInputWidth() const
{

    ImplWinData* pWinData = ImplGetWinData();
    return pWinData->mnCursorExtWidth;
}

void Window::SetCompositionCharRect( const Rectangle* pRect, long nCompositionLength, bool bVertical ) {

    ImplWinData* pWinData = ImplGetWinData();
    delete[] pWinData->mpCompositionCharRects;
    pWinData->mbVertical = bVertical;
    pWinData->mpCompositionCharRects = NULL;
    pWinData->mnCompositionCharRects = nCompositionLength;
    if ( pRect && (nCompositionLength > 0) )
    {
        pWinData->mpCompositionCharRects = new Rectangle[nCompositionLength];
        for (long i = 0; i < nCompositionLength; ++i)
            pWinData->mpCompositionCharRects[i] = pRect[i];
    }
}

void Window::SetSettings( const AllSettings& rSettings )
{
    SetSettings( rSettings, false );
}

void Window::SetSettings( const AllSettings& rSettings, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->SetSettings( rSettings, false );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->SetSettings( rSettings, true );
    }

    AllSettings aOldSettings(*mxSettings);
    OutputDevice::SetSettings( rSettings );
    sal_uLong nChangeFlags = aOldSettings.GetChangeFlags( rSettings );

    // recalculate AppFont-resolution and DPI-resolution
    ImplInitResolutionSettings();

    if ( nChangeFlags )
    {
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
        DataChanged( aDCEvt );
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->SetSettings( rSettings, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::UpdateSettings( const AllSettings& rSettings, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->UpdateSettings( rSettings, false );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->UpdateSettings( rSettings, true );
    }

    AllSettings aOldSettings(*mxSettings);
    sal_uLong nChangeFlags = mxSettings->Update( mxSettings->GetWindowUpdate(), rSettings );
    nChangeFlags |= SETTINGS_IN_UPDATE_SETTINGS; // Set this flag so the receiver of the data changed
                                                 // event can distinguish between the changing of global
                                                 // setting and a local change ( with SetSettings )

    // recalculate AppFont-resolution and DPI-resolution
    ImplInitResolutionSettings();

    /* #i73785#
    *  do not overwrite a WheelBehavior with false
    *  this looks kind of a hack, but WheelBehavior
    *  is always a local change, not a system property,
    *  so we can spare all our users the hassle of reacting on
    *  this in their respective DataChanged.
    */
    MouseSettings aSet( mxSettings->GetMouseSettings() );
    aSet.SetWheelBehavior( aOldSettings.GetMouseSettings().GetWheelBehavior() );
    mxSettings->SetMouseSettings( aSet );

    if( (nChangeFlags & SETTINGS_STYLE) && IsBackground() )
    {
        Wallpaper aWallpaper = GetBackground();
        if( !aWallpaper.IsBitmap() && !aWallpaper.IsGradient() )
        {
            if ( mpWindowImpl->mnStyle & WB_3DLOOK )
            {
                if (aOldSettings.GetStyleSettings().GetFaceColor() != rSettings.GetStyleSettings().GetFaceColor())
                    SetBackground( Wallpaper( rSettings.GetStyleSettings().GetFaceColor() ) );
            }
            else
            {
                if (aOldSettings.GetStyleSettings().GetWindowColor() != rSettings.GetStyleSettings().GetWindowColor())
                    SetBackground( Wallpaper( rSettings.GetStyleSettings().GetWindowColor() ) );
            }
        }
    }

    if ( nChangeFlags )
    {
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, &aOldSettings, nChangeFlags );
        DataChanged( aDCEvt );
        // notify data change handler
        ImplCallEventListeners( VCLEVENT_WINDOW_DATACHANGED, &aDCEvt);
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->UpdateSettings( rSettings, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::NotifyAllChildren( DataChangedEvent& rDCEvt )
{

    DataChanged( rDCEvt );

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->NotifyAllChildren( rDCEvt );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

void Window::SetPointFont( const Font& rFont )
{

    Font aFont = rFont;
    ImplPointToLogic( aFont );
    SetFont( aFont );
}

Font Window::GetPointFont() const
{

    Font aFont = GetFont();
    ImplLogicToPoint( aFont );
    return aFont;
}

void Window::SetWindowRegionPixel()
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetWindowRegionPixel();
    else if( mpWindowImpl->mbFrame )
    {
        mpWindowImpl->maWinRegion = Region(true);
        mpWindowImpl->mbWinRegion = false;
        mpWindowImpl->mpFrame->ResetClipRegion();
    }
    else
    {
        if ( mpWindowImpl->mbWinRegion )
        {
            mpWindowImpl->maWinRegion = Region(true);
            mpWindowImpl->mbWinRegion = false;
            ImplSetClipFlag();

            if ( IsReallyVisible() )
            {
                // restore background storage
                if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mpSaveBackDev )
                    ImplDeleteOverlapBackground();
                if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                    ImplInvalidateAllOverlapBackgrounds();
                Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
                Region      aRegion( aRect );
                ImplInvalidateParentFrameRegion( aRegion );
            }
        }
    }
}

void Window::SetWindowRegionPixel( const Region& rRegion )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetWindowRegionPixel( rRegion );
    else if( mpWindowImpl->mbFrame )
    {
        if( !rRegion.IsNull() )
        {
            mpWindowImpl->maWinRegion = rRegion;
            mpWindowImpl->mbWinRegion = ! rRegion.IsEmpty();

            if( mpWindowImpl->mbWinRegion )
            {
                // set/update ClipRegion
                RectangleVector aRectangles;
                mpWindowImpl->maWinRegion.GetRegionRectangles(aRectangles);
                mpWindowImpl->mpFrame->BeginSetClipRegion(aRectangles.size());

                for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
                {
                    mpWindowImpl->mpFrame->UnionClipRegion(
                        aRectIter->Left(),
                        aRectIter->Top(),
                        aRectIter->GetWidth(),       // orig nWidth was ((R - L) + 1), same as GetWidth does
                        aRectIter->GetHeight());     // same for height
                }

                mpWindowImpl->mpFrame->EndSetClipRegion();

                //long                nX;
                //long                nY;
                //long                nWidth;
                //long                nHeight;
                //sal_uLong               nRectCount;
                //ImplRegionInfo      aInfo;
                //sal_Bool                bRegionRect;

                //nRectCount = mpWindowImpl->maWinRegion.GetRectCount();
                //mpWindowImpl->mpFrame->BeginSetClipRegion( nRectCount );
                //bRegionRect = mpWindowImpl->maWinRegion.ImplGetFirstRect( aInfo, nX, nY, nWidth, nHeight );
                //while ( bRegionRect )
                //{
                //    mpWindowImpl->mpFrame->UnionClipRegion( nX, nY, nWidth, nHeight );
                //    bRegionRect = mpWindowImpl->maWinRegion.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
                //}
                //mpWindowImpl->mpFrame->EndSetClipRegion();
            }
            else
                SetWindowRegionPixel();
        }
        else
            SetWindowRegionPixel();
    }
    else
    {
        if ( rRegion.IsNull() )
        {
            if ( mpWindowImpl->mbWinRegion )
            {
                mpWindowImpl->maWinRegion = Region(true);
                mpWindowImpl->mbWinRegion = false;
                ImplSetClipFlag();
            }
        }
        else
        {
            mpWindowImpl->maWinRegion = rRegion;
            mpWindowImpl->mbWinRegion = true;
            ImplSetClipFlag();
        }

        if ( IsReallyVisible() )
        {
            // restore background storage
            if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mpSaveBackDev )
                ImplDeleteOverlapBackground();
            if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
                ImplInvalidateAllOverlapBackgrounds();
            Rectangle   aRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
            Region      aRegion( aRect );
            ImplInvalidateParentFrameRegion( aRegion );
        }
    }
}

const Region& Window::GetWindowRegionPixel() const
{

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->GetWindowRegionPixel();
    else
        return mpWindowImpl->maWinRegion;
}

bool Window::IsWindowRegionPixel() const
{

    if ( mpWindowImpl->mpBorderWindow )
        return mpWindowImpl->mpBorderWindow->IsWindowRegionPixel();
    else
        return mpWindowImpl->mbWinRegion;
}

Region Window::GetPaintRegion() const
{

    if ( mpWindowImpl->mpPaintRegion )
    {
        Region aRegion = *mpWindowImpl->mpPaintRegion;
        aRegion.Move( -mnOutOffX, -mnOutOffY );
        return PixelToLogic( aRegion );
    }
    else
    {
        Region aPaintRegion(true);
        return aPaintRegion;
    }
}

static SystemWindow *ImplGetLastSystemWindow( Window *pWin )
{
    // get the most top-level system window, the one that contains the taskpanelist
    SystemWindow *pSysWin = NULL;
    if( !pWin )
        return pSysWin;
    Window *pMyParent = pWin;
    while ( pMyParent )
    {
        if ( pMyParent->IsSystemWindow() )
            pSysWin = (SystemWindow*)pMyParent;
        pMyParent = pMyParent->GetParent();
    }
    return pSysWin;
}

void Window::SetParent( Window* pNewParent )
{
    DBG_ASSERT( pNewParent, "Window::SetParent(): pParent == NULL" );
    DBG_ASSERT( pNewParent != this, "someone tried to reparent a window to itself" );

    if( pNewParent == this )
        return;

    // check if the taskpanelist would change and move the window pointer accordingly
    SystemWindow *pSysWin = ImplGetLastSystemWindow(this);
    SystemWindow *pNewSysWin = NULL;
    bool bChangeTaskPaneList = false;
    if( pSysWin && pSysWin->ImplIsInTaskPaneList( this ) )
    {
        pNewSysWin = ImplGetLastSystemWindow( pNewParent );
        if( pNewSysWin && pNewSysWin != pSysWin )
        {
            bChangeTaskPaneList = true;
            pSysWin->GetTaskPaneList()->RemoveWindow( this );
        }
    }
    // remove ownerdraw decorated windows from list in the top-most frame window
    if( (GetStyle() & WB_OWNERDRAWDECORATION) && mpWindowImpl->mbFrame )
    {
        ::std::vector< Window* >& rList = ImplGetOwnerDrawList();
        ::std::vector< Window* >::iterator p;
        p = ::std::find( rList.begin(), rList.end(), this );
        if( p != rList.end() )
            rList.erase( p );
    }

    ImplSetFrameParent( pNewParent );

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpRealParent = pNewParent;
        mpWindowImpl->mpBorderWindow->SetParent( pNewParent );
        return;
    }

    if ( mpWindowImpl->mpParent == pNewParent )
        return;

    if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetParent( pNewParent->mpWindowImpl->mpFrame );

    bool bVisible = IsVisible();
    Show( false, SHOW_NOFOCUSCHANGE );

    // check if the overlap window changes
    Window* pOldOverlapWindow;
    Window* pNewOverlapWindow = NULL;
    if ( ImplIsOverlapWindow() )
        pOldOverlapWindow = NULL;
    else
    {
        pNewOverlapWindow = pNewParent->ImplGetFirstOverlapWindow();
        if ( mpWindowImpl->mpOverlapWindow != pNewOverlapWindow )
            pOldOverlapWindow = mpWindowImpl->mpOverlapWindow;
        else
            pOldOverlapWindow = NULL;
    }

    // convert windows in the hierarchy
    bool bFocusOverlapWin = HasChildPathFocus( true );
    bool bFocusWin = HasChildPathFocus();
    bool bNewFrame = pNewParent->mpWindowImpl->mpFrameWindow != mpWindowImpl->mpFrameWindow;
    if ( bNewFrame )
    {
        if ( mpWindowImpl->mpFrameData->mpFocusWin )
        {
            if ( IsWindowOrChild( mpWindowImpl->mpFrameData->mpFocusWin ) )
                mpWindowImpl->mpFrameData->mpFocusWin = NULL;
        }
        if ( mpWindowImpl->mpFrameData->mpMouseMoveWin )
        {
            if ( IsWindowOrChild( mpWindowImpl->mpFrameData->mpMouseMoveWin ) )
                mpWindowImpl->mpFrameData->mpMouseMoveWin = NULL;
        }
        if ( mpWindowImpl->mpFrameData->mpMouseDownWin )
        {
            if ( IsWindowOrChild( mpWindowImpl->mpFrameData->mpMouseDownWin ) )
                mpWindowImpl->mpFrameData->mpMouseDownWin = NULL;
        }
    }
    ImplRemoveWindow( bNewFrame );
    ImplInsertWindow( pNewParent );
    if ( mpWindowImpl->mnParentClipMode & PARENTCLIPMODE_CLIP )
        pNewParent->mpWindowImpl->mbClipChildren = true;
    ImplUpdateWindowPtr();
    if ( ImplUpdatePos() )
        ImplUpdateSysObjPos();

    // If the Overlap-Window has changed, we need to test whether
    // OverlapWindows that had the Child window as their parent
    // need to be put into the window hierarchy.
    if ( ImplIsOverlapWindow() )
    {
        if ( bNewFrame )
        {
            Window* pOverlapWindow = mpWindowImpl->mpFirstOverlap;
            while ( pOverlapWindow )
            {
                Window* pNextOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
                pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
                pOverlapWindow = pNextOverlapWindow;
            }
        }
    }
    else if ( pOldOverlapWindow )
    {
        // reset Focus-Save
        if ( bFocusWin ||
             (pOldOverlapWindow->mpWindowImpl->mpLastFocusWindow &&
              IsWindowOrChild( pOldOverlapWindow->mpWindowImpl->mpLastFocusWindow )) )
            pOldOverlapWindow->mpWindowImpl->mpLastFocusWindow = NULL;

        Window* pOverlapWindow = pOldOverlapWindow->mpWindowImpl->mpFirstOverlap;
        while ( pOverlapWindow )
        {
            Window* pNextOverlapWindow = pOverlapWindow->mpWindowImpl->mpNext;
            if ( ImplIsRealParentPath( pOverlapWindow->ImplGetWindow() ) )
                pOverlapWindow->ImplUpdateOverlapWindowPtr( bNewFrame );
            pOverlapWindow = pNextOverlapWindow;
        }

        // update activate-status at next overlap window
        if ( HasChildPathFocus( true ) )
            ImplCallFocusChangeActivate( pNewOverlapWindow, pOldOverlapWindow );
    }

    // also convert Activate-Status
    if ( bNewFrame )
    {
        if ( (GetType() == WINDOW_BORDERWINDOW) &&
             (ImplGetWindow()->GetType() == WINDOW_FLOATINGWINDOW) )
            ((ImplBorderWindow*)this)->SetDisplayActive( mpWindowImpl->mpFrameData->mbHasFocus );
    }

    // when required give focus to new frame if
    // FocusWindow is changed with SetParent()
    if ( bFocusOverlapWin )
    {
        mpWindowImpl->mpFrameData->mpFocusWin = Application::GetFocusWindow();
        if ( !mpWindowImpl->mpFrameData->mbHasFocus )
        {
            mpWindowImpl->mpFrame->ToTop( 0 );
        }
    }

    // Assure DragSource and DropTarget members are created
    if ( bNewFrame )
    {
            GetDropTarget();
    }

    if( bChangeTaskPaneList )
        pNewSysWin->GetTaskPaneList()->AddWindow( this );

    if( (GetStyle() & WB_OWNERDRAWDECORATION) && mpWindowImpl->mbFrame )
        ImplGetOwnerDrawList().push_back( this );

    if ( bVisible )
        Show( true, SHOW_NOFOCUSCHANGE | SHOW_NOACTIVATE );
}

void Window::Show( bool bVisible, sal_uInt16 nFlags )
{

    if ( mpWindowImpl->mbVisible == bVisible )
        return;

    ImplDelData aDogTag( this );

    bool bRealVisibilityChanged = false;
    mpWindowImpl->mbVisible = bVisible;

    if ( !bVisible )
    {
        ImplHideAllOverlaps();
        if( aDogTag.IsDead() )
            return;

        if ( mpWindowImpl->mpBorderWindow )
        {
            bool bOldUpdate = mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate;
            if ( mpWindowImpl->mbNoParentUpdate )
                mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate = true;
            mpWindowImpl->mpBorderWindow->Show( false, nFlags );
            mpWindowImpl->mpBorderWindow->mpWindowImpl->mbNoParentUpdate = bOldUpdate;
        }
        else if ( mpWindowImpl->mbFrame )
        {
            mpWindowImpl->mbSuppressAccessibilityEvents = true;
            mpWindowImpl->mpFrame->Show( false, false );
        }

        StateChanged( STATE_CHANGE_VISIBLE );

        if ( mpWindowImpl->mbReallyVisible )
        {
            Region  aInvRegion;
            bool    bSaveBack = false;

            if ( ImplIsOverlapWindow() && !mpWindowImpl->mbFrame )
            {
                if ( ImplRestoreOverlapBackground( aInvRegion ) )
                    bSaveBack = true;
            }

            if ( !bSaveBack )
            {
                if ( mpWindowImpl->mbInitWinClipRegion )
                    ImplInitWinClipRegion();
                aInvRegion = mpWindowImpl->maWinClipRegion;
            }

            if( aDogTag.IsDead() )
                return;

            bRealVisibilityChanged = mpWindowImpl->mbReallyVisible;
            ImplResetReallyVisible();
            ImplSetClipFlag();

            if ( ImplIsOverlapWindow() && !mpWindowImpl->mbFrame )
            {
                // convert focus
                if ( !(nFlags & SHOW_NOFOCUSCHANGE) && HasChildPathFocus() )
                {
                    if ( mpWindowImpl->mpOverlapWindow->IsEnabled() &&
                         mpWindowImpl->mpOverlapWindow->IsInputEnabled() &&
                         ! mpWindowImpl->mpOverlapWindow->IsInModalMode()
                         )
                        mpWindowImpl->mpOverlapWindow->GrabFocus();
                }
            }

            if ( !mpWindowImpl->mbFrame )
            {
                if( mpWindowImpl->mpWinData && mpWindowImpl->mpWinData->mbEnableNativeWidget )
                {
                    /*
                    * #i48371# native theming: some themes draw outside the control
                    * area we tell them to (bad thing, but we cannot do much about it ).
                    * On hiding these controls they get invalidated with their window rectangle
                    * which leads to the parts outside the control area being left and not
                    * invalidated. Workaround: invalidate an area on the parent, too
                    */
                    const int workaround_border = 5;
                    Rectangle aBounds( aInvRegion.GetBoundRect() );
                    aBounds.Left()      -= workaround_border;
                    aBounds.Top()       -= workaround_border;
                    aBounds.Right()     += workaround_border;
                    aBounds.Bottom()    += workaround_border;
                    aInvRegion = aBounds;
                }
                if ( !mpWindowImpl->mbNoParentUpdate && !(nFlags & SHOW_NOPARENTUPDATE) )
                {
                    if ( !aInvRegion.IsEmpty() )
                        ImplInvalidateParentFrameRegion( aInvRegion );
                }
                ImplGenerateMouseMove();
            }
        }
    }
    else
    {
        // inherit native widget flag for form controls
        // required here, because frames never show up in the child hierarchy - which should be fixed....
        // eg, the drop down of a combobox which is a system floating window
        if( mpWindowImpl->mbFrame && GetParent() && GetParent()->IsCompoundControl() &&
            GetParent()->IsNativeWidgetEnabled() != IsNativeWidgetEnabled() )
            EnableNativeWidget( GetParent()->IsNativeWidgetEnabled() );

        if ( mpWindowImpl->mbCallMove )
        {
            ImplCallMove();
        }
        if ( mpWindowImpl->mbCallResize )
        {
            ImplCallResize();
        }

        StateChanged( STATE_CHANGE_VISIBLE );

        Window* pTestParent;
        if ( ImplIsOverlapWindow() )
            pTestParent = mpWindowImpl->mpOverlapWindow;
        else
            pTestParent = ImplGetParent();
        if ( mpWindowImpl->mbFrame || pTestParent->mpWindowImpl->mbReallyVisible )
        {
            // if a window becomes visible, send all child windows a StateChange,
            // such that these can initialise themselves
            ImplCallInitShow();

            // If it is a SystemWindow it automatically pops up on top of
            // all other windows if needed.
            if ( ImplIsOverlapWindow() && !(nFlags & SHOW_NOACTIVATE) )
            {
                ImplStartToTop(( nFlags & SHOW_FOREGROUNDTASK ) ? TOTOP_FOREGROUNDTASK : 0 );
                ImplFocusToTop( 0, false );
            }

            // save background
            if ( mpWindowImpl->mpOverlapData && mpWindowImpl->mpOverlapData->mbSaveBack )
                ImplSaveOverlapBackground();
            // adjust mpWindowImpl->mbReallyVisible
            bRealVisibilityChanged = !mpWindowImpl->mbReallyVisible;
            ImplSetReallyVisible();

            // assure clip rectangles will be recalculated
            ImplSetClipFlag();

            if ( !mpWindowImpl->mbFrame )
            {
                sal_uInt16 nInvalidateFlags = INVALIDATE_CHILDREN;
                if( ! IsPaintTransparent() )
                    nInvalidateFlags |= INVALIDATE_NOTRANSPARENT;
                ImplInvalidate( NULL, nInvalidateFlags );
                ImplGenerateMouseMove();
            }
        }

        if ( mpWindowImpl->mpBorderWindow )
            mpWindowImpl->mpBorderWindow->Show( true, nFlags );
        else if ( mpWindowImpl->mbFrame )
        {
            // #106431#, hide SplashScreen
            ImplSVData* pSVData = ImplGetSVData();
            if ( !pSVData->mpIntroWindow )
            {
                // The right way would be just to call this (not even in the 'if')
                GetpApp()->InitFinished();
            }
            else if ( !ImplIsWindowOrChild( pSVData->mpIntroWindow ) )
            {
                // ... but the VCL splash is broken, and it needs this
                // (for ./soffice slot:5500)
                pSVData->mpIntroWindow->Hide();
            }

            //DBG_ASSERT( !mpWindowImpl->mbSuppressAccessibilityEvents, "Window::Show() - Frame reactivated");
            mpWindowImpl->mbSuppressAccessibilityEvents = false;

            mpWindowImpl->mbPaintFrame = true;
            if (!Application::GetSettings().GetMiscSettings().GetPseudoHeadless())
            {
                bool bNoActivate = (nFlags & (SHOW_NOACTIVATE|SHOW_NOFOCUSCHANGE)) ? true : false;
                mpWindowImpl->mpFrame->Show( true, bNoActivate );
            }
            if( aDogTag.IsDead() )
                return;

            // Query the correct size of the window, if we are waiting for
            // a system resize
            if ( mpWindowImpl->mbWaitSystemResize )
            {
                long nOutWidth;
                long nOutHeight;
                mpWindowImpl->mpFrame->GetClientSize( nOutWidth, nOutHeight );
                ImplHandleResize( this, nOutWidth, nOutHeight );
            }
        }

        if( aDogTag.IsDead() )
            return;

#if OSL_DEBUG_LEVEL > 0
        if ( IsDialog() || (GetType() == WINDOW_TABPAGE) || (GetType() == WINDOW_DOCKINGWINDOW) )
        {
            DBG_DIALOGTEST( this );
        }
#endif

        ImplShowAllOverlaps();
    }

    if( aDogTag.IsDead() )
        return;
    // invalidate all saved backgrounds
    if ( mpWindowImpl->mpFrameData->mpFirstBackWin )
        ImplInvalidateAllOverlapBackgrounds();

    // the SHOW/HIDE events also serve as indicators to send child creation/destroy events to the access bridge
    // However, the access bridge only uses this event if the data member is not NULL (it's kind of a hack that
    // we re-use the SHOW/HIDE events this way, with this particular semantics).
    // Since #104887#, the notifications for the access bridge are done in Impl(Set|Reset)ReallyVisible. Here, we
    // now only notify with a NULL data pointer, for all other clients except the access bridge.
    if ( !bRealVisibilityChanged )
        ImplCallEventListeners( mpWindowImpl->mbVisible ? VCLEVENT_WINDOW_SHOW : VCLEVENT_WINDOW_HIDE, NULL );
    if( aDogTag.IsDead() )
        return;

    // #107575#, if a floating windows is shown that grabs the focus, we have to notify the toolkit about it
    // ImplGrabFocus() is not called in this case
    // Because this might lead to problems the task will be shifted to 6.y
    // Note: top-level context menus are registered at the access bridge after being shown,
    // so this will probably not help here....
    /*
    if( mpWindowImpl->mbFloatWin && ((FloatingWindow*) this )->GrabsFocus() )
    {
        ImplSVData* pSVData = ImplGetSVData();
        if( !mpWindowImpl->mbVisible )
        {
            ImplCallEventListeners( VCLEVENT_WINDOW_LOSEFOCUS );
            if( pSVData->maWinData.mpFocusWin )
                pSVData->maWinData.mpFocusWin->ImplCallEventListeners( VCLEVENT_WINDOW_GETFOCUS );
        }
        else
        {
            if( pSVData->maWinData.mpFocusWin )
                pSVData->maWinData.mpFocusWin->ImplCallEventListeners( VCLEVENT_WINDOW_LOSEFOCUS );
            ImplCallEventListeners( VCLEVENT_WINDOW_GETFOCUS );
        }
    }
    */
}

Size Window::GetSizePixel() const
{
    if (!mpWindowImpl)
    {
        SAL_WARN("vcl.layout", "WTF no windowimpl");
        return Size(0,0);
    }

    // #i43257# trigger pending resize handler to assure correct window sizes
    if( mpWindowImpl->mpFrameData->maResizeTimer.IsActive() )
    {
        ImplDelData aDogtag( this );
        mpWindowImpl->mpFrameData->maResizeTimer.Stop();
        mpWindowImpl->mpFrameData->maResizeTimer.GetTimeoutHdl().Call( NULL );
        if( aDogtag.IsDead() )
            return Size(0,0);
    }

    return Size( mnOutWidth+mpWindowImpl->mnLeftBorder+mpWindowImpl->mnRightBorder,
                 mnOutHeight+mpWindowImpl->mnTopBorder+mpWindowImpl->mnBottomBorder );
}

void Window::GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                               sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const
{
    rLeftBorder     = mpWindowImpl->mnLeftBorder;
    rTopBorder      = mpWindowImpl->mnTopBorder;
    rRightBorder    = mpWindowImpl->mnRightBorder;
    rBottomBorder   = mpWindowImpl->mnBottomBorder;
}

void Window::Enable( bool bEnable, bool bChild )
{

    if ( !bEnable )
    {
        // the tracking mode will be stopped or the capture will be stolen
        // when a window is disabled,
        if ( IsTracking() )
            EndTracking( ENDTRACK_CANCEL );
        if ( IsMouseCaptured() )
            ReleaseMouse();
        // try to pass focus to the next control
        // if the window has focus and is contained in the dialog control
        // mpWindowImpl->mbDisabled should only be set after a call of ImplDlgCtrlNextWindow().
        // Otherwise ImplDlgCtrlNextWindow() should be used
        if ( HasFocus() )
            ImplDlgCtrlNextWindow();
    }

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->Enable( bEnable, false );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->Enable( bEnable, true );
    }

    // #i56102# restore app focus win in case the
    // window was disabled when the frame focus changed
    ImplSVData* pSVData = ImplGetSVData();
    if( bEnable &&
        pSVData->maWinData.mpFocusWin == NULL &&
        mpWindowImpl->mpFrameData->mbHasFocus &&
        mpWindowImpl->mpFrameData->mpFocusWin == this )
        pSVData->maWinData.mpFocusWin = this;

    if ( mpWindowImpl->mbDisabled != !bEnable )
    {
        mpWindowImpl->mbDisabled = !bEnable;
        if ( mpWindowImpl->mpSysObj )
            mpWindowImpl->mpSysObj->Enable( bEnable && !mpWindowImpl->mbInputDisabled );
        StateChanged( STATE_CHANGE_ENABLE );

        ImplCallEventListeners( bEnable ? VCLEVENT_WINDOW_ENABLED : VCLEVENT_WINDOW_DISABLED );
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->Enable( bEnable, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }

    if ( IsReallyVisible() )
        ImplGenerateMouseMove();
}

void Window::SetCallHandlersOnInputDisabled( bool bCall )
{
    mpWindowImpl->mbCallHandlersDuringInputDisabled = bCall ? sal_True : sal_False;

    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        pChild->SetCallHandlersOnInputDisabled( bCall );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

bool Window::IsCallHandlersOnInputDisabled() const
{
    return mpWindowImpl->mbCallHandlersDuringInputDisabled;
}

void Window::EnableInput( bool bEnable, bool bChild )
{

    bool bNotify = (bEnable != mpWindowImpl->mbInputDisabled);
    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->EnableInput( bEnable, false );
        if ( (mpWindowImpl->mpBorderWindow->GetType() == WINDOW_BORDERWINDOW) &&
             ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow )
            ((ImplBorderWindow*)mpWindowImpl->mpBorderWindow)->mpMenuBarWindow->EnableInput( bEnable, true );
    }

    if ( (! bEnable && mpWindowImpl->meAlwaysInputMode != AlwaysInputEnabled) ||
         (  bEnable && mpWindowImpl->meAlwaysInputMode != AlwaysInputDisabled) )
    {
        // automatically stop the tracking mode or steal capture
        // if the window is disabled
        if ( !bEnable )
        {
            if ( IsTracking() )
                EndTracking( ENDTRACK_CANCEL );
            if ( IsMouseCaptured() )
                ReleaseMouse();
        }

        if ( mpWindowImpl->mbInputDisabled != !bEnable )
        {
            mpWindowImpl->mbInputDisabled = !bEnable;
            if ( mpWindowImpl->mpSysObj )
                mpWindowImpl->mpSysObj->Enable( !mpWindowImpl->mbDisabled && bEnable );
        }
    }

    // #i56102# restore app focus win in case the
    // window was disabled when the frame focus changed
    ImplSVData* pSVData = ImplGetSVData();
    if( bEnable &&
        pSVData->maWinData.mpFocusWin == NULL &&
        mpWindowImpl->mpFrameData->mbHasFocus &&
        mpWindowImpl->mpFrameData->mpFocusWin == this )
        pSVData->maWinData.mpFocusWin = this;

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->EnableInput( bEnable, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }

    if ( IsReallyVisible() )
        ImplGenerateMouseMove();

    // #104827# notify parent
    if ( bNotify )
    {
        NotifyEvent aNEvt( bEnable ? EVENT_INPUTENABLE : EVENT_INPUTDISABLE, this );
        Notify( aNEvt );
    }
}

void Window::EnableInput( bool bEnable, bool bChild, bool bSysWin,
                          const Window* pExcludeWindow )
{

    EnableInput( bEnable, bChild );
    if ( bSysWin )
    {
        // pExculeWindow is the first Overlap-Frame --> if this
        // shouldn't be the case, than this must be changed in dialog.cxx
        if( pExcludeWindow )
            pExcludeWindow = pExcludeWindow->ImplGetFirstOverlapWindow();
        Window* pSysWin = mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrameData->mpFirstOverlap;
        while ( pSysWin )
        {
            // Is Window in the path from this window
            if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( pSysWin, true ) )
            {
                // Is Window not in the exclude window path or not the
                // exclude window, than change the status
                if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( pSysWin, true ) )
                    pSysWin->EnableInput( bEnable, bChild );
            }
            pSysWin = pSysWin->mpWindowImpl->mpNextOverlap;
        }

        // enable/disable floating system windows as well
        Window* pFrameWin = ImplGetSVData()->maWinData.mpFirstFrame;
        while ( pFrameWin )
        {
            if( pFrameWin->ImplIsFloatingWindow() )
            {
                // Is Window in the path from this window
                if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( pFrameWin, true ) )
                {
                    // Is Window not in the exclude window path or not the
                    // exclude window, than change the status
                    if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( pFrameWin, true ) )
                        pFrameWin->EnableInput( bEnable, bChild );
                }
            }
            pFrameWin = pFrameWin->mpWindowImpl->mpFrameData->mpNextFrame;
        }

        // the same for ownerdraw floating windows
        if( mpWindowImpl->mbFrame )
        {
            ::std::vector< Window* >& rList = mpWindowImpl->mpFrameData->maOwnerDrawList;
            ::std::vector< Window* >::iterator p = rList.begin();
            while( p != rList.end() )
            {
                // Is Window in the path from this window
                if ( ImplGetFirstOverlapWindow()->ImplIsWindowOrChild( (*p), true ) )
                {
                    // Is Window not in the exclude window path or not the
                    // exclude window, than change the status
                    if ( !pExcludeWindow || !pExcludeWindow->ImplIsWindowOrChild( (*p), true ) )
                        (*p)->EnableInput( bEnable, bChild );
                }
                ++p;
            }
        }
    }
}

void Window::AlwaysEnableInput( bool bAlways, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->AlwaysEnableInput( bAlways, false );

    if( bAlways && mpWindowImpl->meAlwaysInputMode != AlwaysInputEnabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputEnabled;

        if ( bAlways )
            EnableInput( true, false );
    }
    else if( ! bAlways && mpWindowImpl->meAlwaysInputMode == AlwaysInputEnabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputNone;
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->AlwaysEnableInput( bAlways, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::AlwaysDisableInput( bool bAlways, bool bChild )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->AlwaysDisableInput( bAlways, false );

    if( bAlways && mpWindowImpl->meAlwaysInputMode != AlwaysInputDisabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputDisabled;

        if ( bAlways )
            EnableInput( false, false );
    }
    else if( ! bAlways && mpWindowImpl->meAlwaysInputMode == AlwaysInputDisabled )
    {
        mpWindowImpl->meAlwaysInputMode = AlwaysInputNone;
    }

    if ( bChild || mpWindowImpl->mbChildNotify )
    {
        Window* pChild = mpWindowImpl->mpFirstChild;
        while ( pChild )
        {
            pChild->AlwaysDisableInput( bAlways, bChild );
            pChild = pChild->mpWindowImpl->mpNext;
        }
    }
}

void Window::SetActivateMode( sal_uInt16 nMode )
{

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetActivateMode( nMode );

    if ( mpWindowImpl->mnActivateMode != nMode )
    {
        mpWindowImpl->mnActivateMode = nMode;

        // possibly trigger Decativate/Activate
        if ( mpWindowImpl->mnActivateMode )
        {
            if ( (mpWindowImpl->mbActive || (GetType() == WINDOW_BORDERWINDOW)) &&
                 !HasChildPathFocus( true ) )
            {
                mpWindowImpl->mbActive = false;
                Deactivate();
            }
        }
        else
        {
            if ( !mpWindowImpl->mbActive || (GetType() == WINDOW_BORDERWINDOW) )
            {
                mpWindowImpl->mbActive = true;
                Activate();
            }
        }
    }
}

void Window::setPosSizePixel( long nX, long nY,
                              long nWidth, long nHeight, sal_uInt16 nFlags )
{

    bool bHasValidSize = !mpWindowImpl->mbDefSize;

    if ( nFlags & WINDOW_POSSIZE_POS )
        mpWindowImpl->mbDefPos = false;
    if ( nFlags & WINDOW_POSSIZE_SIZE )
        mpWindowImpl->mbDefSize = false;

    // The top BorderWindow is the window which is to be positioned
    Window* pWindow = this;
    while ( pWindow->mpWindowImpl->mpBorderWindow )
        pWindow = pWindow->mpWindowImpl->mpBorderWindow;

    if ( pWindow->mpWindowImpl->mbFrame )
    {
        // Note: if we're positioning a frame, the coordinates are interpreted
        // as being the top-left corner of the window's client area and NOT
        // as the position of the border ! (due to limitations of several UNIX window managers)
        long nOldWidth  = pWindow->mnOutWidth;

        if ( !(nFlags & WINDOW_POSSIZE_WIDTH) )
            nWidth = pWindow->mnOutWidth;
        if ( !(nFlags & WINDOW_POSSIZE_HEIGHT) )
            nHeight = pWindow->mnOutHeight;

        sal_uInt16 nSysFlags=0;
        Window *pParent = GetParent();

        if( nFlags & WINDOW_POSSIZE_WIDTH )
            nSysFlags |= SAL_FRAME_POSSIZE_WIDTH;
        if( nFlags & WINDOW_POSSIZE_HEIGHT )
            nSysFlags |= SAL_FRAME_POSSIZE_HEIGHT;
        if( nFlags & WINDOW_POSSIZE_X )
        {
            nSysFlags |= SAL_FRAME_POSSIZE_X;
            if( pParent && (pWindow->GetStyle() & WB_SYSTEMCHILDWINDOW) )
            {
                nX += pParent->mnOutOffX;
            }
            if( pParent && pParent->ImplIsAntiparallel() )
            {
                // --- RTL --- (re-mirror at parent window)
                Rectangle aRect( Point ( nX, nY ), Size( nWidth, nHeight ) );
                const OutputDevice *pParentOutDev = pParent->GetOutDev();
                pParentOutDev->ReMirror( aRect );
                nX = aRect.Left();
            }
        }
        if( !(nFlags & WINDOW_POSSIZE_X) && bHasValidSize && pWindow->mpWindowImpl->mpFrame->maGeometry.nWidth )
        {
            // --- RTL ---  make sure the old right aligned position is not changed
            //              system windows will always grow to the right
            if ( pParent )
            {
                OutputDevice *pParentOutDev = pParent->GetOutDev();
                if( pParentOutDev->HasMirroredGraphics() )
                {
                    long myWidth = nOldWidth;
                    if( !myWidth )
                        myWidth = mpWindowImpl->mpFrame->GetUnmirroredGeometry().nWidth;
                    if( !myWidth )
                        myWidth = nWidth;
                    nFlags |= WINDOW_POSSIZE_X;
                    nSysFlags |= SAL_FRAME_POSSIZE_X;
                    nX = mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX - pParent->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX -
                        mpWindowImpl->mpFrame->GetUnmirroredGeometry().nLeftDecoration;
                    nX = pParent->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX - mpWindowImpl->mpFrame->GetUnmirroredGeometry().nLeftDecoration +
                        pParent->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nWidth - myWidth - 1 - mpWindowImpl->mpFrame->GetUnmirroredGeometry().nX;
                    if(!(nFlags & WINDOW_POSSIZE_Y))
                    {
                        nFlags |= WINDOW_POSSIZE_Y;
                        nSysFlags |= SAL_FRAME_POSSIZE_Y;
                        nY = mpWindowImpl->mpFrame->GetUnmirroredGeometry().nY - pWindow->GetParent()->mpWindowImpl->mpFrame->GetUnmirroredGeometry().nY -
                            mpWindowImpl->mpFrame->GetUnmirroredGeometry().nTopDecoration;
                    }
                }
            }
        }
        if( nFlags & WINDOW_POSSIZE_Y )
        {
            nSysFlags |= SAL_FRAME_POSSIZE_Y;
            if( pParent && (pWindow->GetStyle() & WB_SYSTEMCHILDWINDOW) )
            {
                nY += pParent->mnOutOffY;
            }
        }

        if( nSysFlags & (SAL_FRAME_POSSIZE_WIDTH|SAL_FRAME_POSSIZE_HEIGHT) )
        {
            // check for min/max client size and adjust size accordingly
            // otherwise it may happen that the resize event is ignored, i.e. the old size remains
            // unchanged but ImplHandleResize() is called with the wrong size
            SystemWindow *pSystemWindow = dynamic_cast< SystemWindow* >( pWindow );
            if( pSystemWindow )
            {
                Size aMinSize = pSystemWindow->GetMinOutputSizePixel();
                Size aMaxSize = pSystemWindow->GetMaxOutputSizePixel();
                if( nWidth < aMinSize.Width() )
                    nWidth = aMinSize.Width();
                if( nHeight < aMinSize.Height() )
                    nHeight = aMinSize.Height();

                if( nWidth > aMaxSize.Width() )
                    nWidth = aMaxSize.Width();
                if( nHeight > aMaxSize.Height() )
                    nHeight = aMaxSize.Height();
            }
        }

        pWindow->mpWindowImpl->mpFrame->SetPosSize( nX, nY, nWidth, nHeight, nSysFlags );

        // Resize should be called directly. If we havn't
        // set the correct size, we get a second resize from
        // the system with the correct size. This can be happened
        // if the size is to small or to large.
        ImplHandleResize( pWindow, nWidth, nHeight );
    }
    else
    {
        pWindow->ImplPosSizeWindow( nX, nY, nWidth, nHeight, nFlags );
        if ( IsReallyVisible() )
            ImplGenerateMouseMove();
    }
}

Point Window::GetPosPixel() const
{
    return mpWindowImpl->maPos;
}

Rectangle Window::GetDesktopRectPixel() const
{
    Rectangle rRect;
    mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrame->GetWorkArea( rRect );
    return rRect;
}

Point Window::OutputToScreenPixel( const Point& rPos ) const
{
    // relative to top level parent
    return Point( rPos.X()+mnOutOffX, rPos.Y()+mnOutOffY );
}

Point Window::ScreenToOutputPixel( const Point& rPos ) const
{
    // relative to top level parent
    return Point( rPos.X()-mnOutOffX, rPos.Y()-mnOutOffY );
}

long Window::ImplGetUnmirroredOutOffX()
{
    // revert mnOutOffX changes that were potentially made in ImplPosSizeWindow
    long offx = mnOutOffX;
    OutputDevice *pOutDev = GetOutDev();
    if( pOutDev->HasMirroredGraphics() )
    {
        if( mpWindowImpl->mpParent && !mpWindowImpl->mpParent->mpWindowImpl->mbFrame && mpWindowImpl->mpParent->ImplIsAntiparallel() )
        {
            if ( !ImplIsOverlapWindow() )
                offx -= mpWindowImpl->mpParent->mnOutOffX;

            offx = mpWindowImpl->mpParent->mnOutWidth - mnOutWidth - offx;

            if ( !ImplIsOverlapWindow() )
                offx += mpWindowImpl->mpParent->mnOutOffX;

        }
    }
    return offx;
}

// normalized screen pixel are independent of mirroring
Point Window::OutputToNormalizedScreenPixel( const Point& rPos ) const
{
    // relative to top level parent
    long offx = ((Window*) this)->ImplGetUnmirroredOutOffX();
    return Point( rPos.X()+offx, rPos.Y()+mnOutOffY );
}

Point Window::NormalizedScreenToOutputPixel( const Point& rPos ) const
{
    // relative to top level parent
    long offx = ((Window*) this)->ImplGetUnmirroredOutOffX();
    return Point( rPos.X()-offx, rPos.Y()-mnOutOffY );
}

Point Window::OutputToAbsoluteScreenPixel( const Point& rPos ) const
{
    // relative to the screen
    Point p = OutputToScreenPixel( rPos );
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    p.X() += g.nX;
    p.Y() += g.nY;
    return p;
}

Point Window::AbsoluteScreenToOutputPixel( const Point& rPos ) const
{
    // relative to the screen
    Point p = ScreenToOutputPixel( rPos );
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    p.X() -= g.nX;
    p.Y() -= g.nY;
    return p;
}

Rectangle Window::ImplOutputToUnmirroredAbsoluteScreenPixel( const Rectangle &rRect ) const
{
    // this method creates unmirrored screen coordinates to be compared with the desktop
    // and is used for positioning of RTL popup windows correctly on the screen
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetUnmirroredGeometry();

    Point p1 = OutputToScreenPixel( rRect.TopRight() );
    p1.X() = g.nX+g.nWidth-p1.X();
    p1.Y() += g.nY;

    Point p2 = OutputToScreenPixel( rRect.BottomLeft() );
    p2.X() = g.nX+g.nWidth-p2.X();
    p2.Y() += g.nY;

    return Rectangle( p1, p2 );
}

Rectangle Window::GetWindowExtentsRelative( Window *pRelativeWindow ) const
{
    // with decoration
    return ImplGetWindowExtentsRelative( pRelativeWindow, false );
}

Rectangle Window::GetClientWindowExtentsRelative( Window *pRelativeWindow ) const
{
    // without decoration
    return ImplGetWindowExtentsRelative( pRelativeWindow, true );
}

Rectangle Window::ImplGetWindowExtentsRelative( Window *pRelativeWindow, bool bClientOnly ) const
{
    SalFrameGeometry g = mpWindowImpl->mpFrame->GetGeometry();
    // make sure we use the extent of our border window,
    // otherwise we miss a few pixels
    const Window *pWin = (!bClientOnly && mpWindowImpl->mpBorderWindow) ? mpWindowImpl->mpBorderWindow : this;

    Point aPos( pWin->OutputToScreenPixel( Point(0,0) ) );
    aPos.X() += g.nX;
    aPos.Y() += g.nY;
    Size aSize ( pWin->GetSizePixel() );
    // #104088# do not add decoration to the workwindow to be compatible to java accessibility api
    if( !bClientOnly && (mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame && GetType() != WINDOW_WORKWINDOW)) )
    {
        aPos.X() -= g.nLeftDecoration;
        aPos.Y() -= g.nTopDecoration;
        aSize.Width() += g.nLeftDecoration + g.nRightDecoration;
        aSize.Height() += g.nTopDecoration + g.nBottomDecoration;
    }
    if( pRelativeWindow )
    {
        // #106399# express coordinates relative to borderwindow
        Window *pRelWin = (!bClientOnly && pRelativeWindow->mpWindowImpl->mpBorderWindow) ? pRelativeWindow->mpWindowImpl->mpBorderWindow : pRelativeWindow;
        aPos = pRelWin->AbsoluteScreenToOutputPixel( aPos );
    }
    return Rectangle( aPos, aSize );
}

void Window::Scroll( long nHorzScroll, long nVertScroll, sal_uInt16 nFlags )
{

    ImplScroll( Rectangle( Point( mnOutOffX, mnOutOffY ),
                           Size( mnOutWidth, mnOutHeight ) ),
                nHorzScroll, nVertScroll, nFlags & ~SCROLL_CLIP );
}

void Window::Scroll( long nHorzScroll, long nVertScroll,
                     const Rectangle& rRect, sal_uInt16 nFlags )
{

    OutputDevice *pOutDev = GetOutDev();
    Rectangle aRect = pOutDev->ImplLogicToDevicePixel( rRect );
    aRect.Intersection( Rectangle( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) ) );
    if ( !aRect.IsEmpty() )
        ImplScroll( aRect, nHorzScroll, nVertScroll, nFlags );
}

void Window::Invalidate( sal_uInt16 nFlags )
{

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    ImplInvalidate( NULL, nFlags );
}

void Window::Invalidate( const Rectangle& rRect, sal_uInt16 nFlags )
{

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    OutputDevice *pOutDev = GetOutDev();
    Rectangle aRect = pOutDev->ImplLogicToDevicePixel( rRect );
    if ( !aRect.IsEmpty() )
    {
        Region aRegion( aRect );
        ImplInvalidate( &aRegion, nFlags );
    }
}

void Window::Invalidate( const Region& rRegion, sal_uInt16 nFlags )
{

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    if ( rRegion.IsNull() )
        ImplInvalidate( NULL, nFlags );
    else
    {
        Region aRegion = ImplPixelToDevicePixel( LogicToPixel( rRegion ) );
        if ( !aRegion.IsEmpty() )
            ImplInvalidate( &aRegion, nFlags );
    }
}

void Window::Validate( sal_uInt16 nFlags )
{

    if ( !IsDeviceOutputNecessary() || !mnOutWidth || !mnOutHeight )
        return;

    ImplValidate( NULL, nFlags );
}

bool Window::HasPaintEvent() const
{

    if ( !mpWindowImpl->mbReallyVisible )
        return false;

    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
        return true;

    if ( mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINT )
        return true;

    if ( !ImplIsOverlapWindow() )
    {
        const Window* pTempWindow = this;
        do
        {
            pTempWindow = pTempWindow->ImplGetParent();
            if ( pTempWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINTCHILDREN | IMPL_PAINT_PAINTALLCHILDREN) )
                return true;
        }
        while ( !pTempWindow->ImplIsOverlapWindow() );
    }

    return false;
}

void Window::Update()
{

    if ( mpWindowImpl->mpBorderWindow )
    {
        mpWindowImpl->mpBorderWindow->Update();
        return;
    }

    if ( !mpWindowImpl->mbReallyVisible )
        return;

    bool bFlush = false;
    if ( mpWindowImpl->mpFrameWindow->mpWindowImpl->mbPaintFrame )
    {
        Point aPoint( 0, 0 );
        Region aRegion( Rectangle( aPoint, Size( mnOutWidth, mnOutHeight ) ) );
        ImplInvalidateOverlapFrameRegion( aRegion );
        if ( mpWindowImpl->mbFrame || (mpWindowImpl->mpBorderWindow && mpWindowImpl->mpBorderWindow->mpWindowImpl->mbFrame) )
            bFlush = true;
    }

    // First we should skip all windows which are Paint-Transparent
    Window* pUpdateWindow = this;
    Window* pWindow = pUpdateWindow;
    while ( !pWindow->ImplIsOverlapWindow() )
    {
        if ( !pWindow->mpWindowImpl->mbPaintTransparent )
        {
            pUpdateWindow = pWindow;
            break;
        }
        pWindow = pWindow->ImplGetParent();
    }
    // In order to limit drawing, an update only draws the window which
    // has PAINTALLCHILDREN set
    pWindow = pUpdateWindow;
    do
    {
        if ( pWindow->mpWindowImpl->mnPaintFlags & IMPL_PAINT_PAINTALLCHILDREN )
            pUpdateWindow = pWindow;
        if ( pWindow->ImplIsOverlapWindow() )
            break;
        pWindow = pWindow->ImplGetParent();
    }
    while ( pWindow );

    // if there is something to paint, trigger a Paint
    if ( pUpdateWindow->mpWindowImpl->mnPaintFlags & (IMPL_PAINT_PAINT | IMPL_PAINT_PAINTCHILDREN) )
    {
        ImplDelData aDogTag(this);

        // trigger an update also for system windows on top of us,
        // otherwise holes would remain
         Window* pUpdateOverlapWindow = ImplGetFirstOverlapWindow()->mpWindowImpl->mpFirstOverlap;
         while ( pUpdateOverlapWindow )
         {
             pUpdateOverlapWindow->Update();
             pUpdateOverlapWindow = pUpdateOverlapWindow->mpWindowImpl->mpNext;
         }

        pUpdateWindow->ImplCallPaint( NULL, pUpdateWindow->mpWindowImpl->mnPaintFlags );

        if (aDogTag.IsDead())
           return;
        bFlush = true;
    }

    if ( bFlush )
        Flush();
}

void Window::Flush()
{

    const Rectangle aWinRect( Point( mnOutOffX, mnOutOffY ), Size( mnOutWidth, mnOutHeight ) );
    mpWindowImpl->mpFrame->Flush( aWinRect );
}

void Window::Sync()
{

    mpWindowImpl->mpFrame->Sync();
}

void Window::SetUpdateMode( bool bUpdate )
{

    mpWindowImpl->mbNoUpdate = !bUpdate;
    StateChanged( STATE_CHANGE_UPDATEMODE );
}

void Window::GrabFocus()
{

    ImplGrabFocus( 0 );
}

bool Window::HasFocus() const
{

    // #107575# the first floating window always has the keyboard focus, see also winproc.cxx: ImplGetKeyInputWindow()
    //  task was shifted to 6.y, so its commented out
    /*
    Window* pFocusWin = ImplGetSVData()->maWinData.mpFirstFloat;
    if( pFocusWin && pFocusWin->mpWindowImpl->mbFloatWin && ((FloatingWindow *)pFocusWin)->GrabsFocus() )
        pFocusWin = pFocusWin->GetPreferredKeyInputWindow();
    else
        pFocusWin = ImplGetSVData()->maWinData.mpFocusWin;

    return (this == pFocusWin);
    */

    return (this == ImplGetSVData()->maWinData.mpFocusWin);
}

void Window::GrabFocusToDocument()
{
    ImplGrabFocusToDocument(0);
}

void Window::SetFakeFocus( bool bFocus )
{
    ImplGetWindowImpl()->mbFakeFocusSet = bFocus;
}

bool Window::HasChildPathFocus( bool bSystemWindow ) const
{

    // #107575#, the first floating window always has the keyboard focus, see also winproc.cxx: ImplGetKeyInputWindow()
    //  task was shifted to 6.y, so its commented out
    /*
    Window* pFocusWin = ImplGetSVData()->maWinData.mpFirstFloat;
    if( pFocusWin && pFocusWin->mpWindowImpl->mbFloatWin && ((FloatingWindow *)pFocusWin)->GrabsFocus() )
        pFocusWin = pFocusWin->GetPreferredKeyInputWindow();
    else
        pFocusWin = ImplGetSVData()->maWinData.mpFocusWin;
    */
    Window* pFocusWin = ImplGetSVData()->maWinData.mpFocusWin;
    if ( pFocusWin )
        return ImplIsWindowOrChild( pFocusWin, bSystemWindow );
    return false;
}

void Window::CaptureMouse()
{

    ImplSVData* pSVData = ImplGetSVData();

    // possibly stop tracking
    if ( pSVData->maWinData.mpTrackWin != this )
    {
        if ( pSVData->maWinData.mpTrackWin )
            pSVData->maWinData.mpTrackWin->EndTracking( ENDTRACK_CANCEL );
    }

    if ( pSVData->maWinData.mpCaptureWin != this )
    {
        pSVData->maWinData.mpCaptureWin = this;
        mpWindowImpl->mpFrame->CaptureMouse( true );
    }
}

void Window::ReleaseMouse()
{

    ImplSVData* pSVData = ImplGetSVData();

    DBG_ASSERTWARNING( pSVData->maWinData.mpCaptureWin == this,
                       "Window::ReleaseMouse(): window doesn't have the mouse capture" );

    if ( pSVData->maWinData.mpCaptureWin == this )
    {
        pSVData->maWinData.mpCaptureWin = NULL;
        mpWindowImpl->mpFrame->CaptureMouse( false );
        ImplGenerateMouseMove();
    }
}

bool Window::IsMouseCaptured() const
{

    return (this == ImplGetSVData()->maWinData.mpCaptureWin);
}

void Window::SetPointer( const Pointer& rPointer )
{

    if ( mpWindowImpl->maPointer == rPointer )
        return;

    mpWindowImpl->maPointer   = rPointer;

    // possibly immediately move pointer
    if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
}

void Window::EnableChildPointerOverwrite( bool bOverwrite )
{

    if ( mpWindowImpl->mbChildPtrOverwrite == bOverwrite )
        return;

    mpWindowImpl->mbChildPtrOverwrite  = bOverwrite;

    // possibly immediately move pointer
    if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
        mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
}

void Window::SetPointerPosPixel( const Point& rPos )
{
    Point aPos = ImplOutputToFrame( rPos );
    const OutputDevice *pOutDev = GetOutDev();
    if( pOutDev->HasMirroredGraphics() )
    {
        if( !IsRTLEnabled() )
        {
            // --- RTL --- (re-mirror mouse pos at this window)
            pOutDev->ReMirror( aPos );
        }
        // mirroring is required here, SetPointerPos bypasses SalGraphics
        mpGraphics->mirror( aPos.X(), this );
    }
    else if( ImplIsAntiparallel() )
    {
        pOutDev->ReMirror( aPos );
    }
    mpWindowImpl->mpFrame->SetPointerPos( aPos.X(), aPos.Y() );
}

Point Window::GetPointerPosPixel()
{

    Point aPos( mpWindowImpl->mpFrameData->mnLastMouseX, mpWindowImpl->mpFrameData->mnLastMouseY );
    if( ImplIsAntiparallel() )
    {
        // --- RTL --- (re-mirror mouse pos at this window)
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aPos );
    }
    return ImplFrameToOutput( aPos );
}

Point Window::GetLastPointerPosPixel()
{

    Point aPos( mpWindowImpl->mpFrameData->mnBeforeLastMouseX, mpWindowImpl->mpFrameData->mnBeforeLastMouseY );
    if( ImplIsAntiparallel() )
    {
        // --- RTL --- (re-mirror mouse pos at this window)
        const OutputDevice *pOutDev = GetOutDev();
        pOutDev->ReMirror( aPos );
    }
    return ImplFrameToOutput( aPos );
}

void Window::ShowPointer( bool bVisible )
{

    if ( mpWindowImpl->mbNoPtrVisible != !bVisible )
    {
        mpWindowImpl->mbNoPtrVisible = !bVisible;

        // possibly immediately move pointer
        if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
    }
}

Window::PointerState Window::GetPointerState()
{
    PointerState aState;
    aState.mnState = 0;

    if (mpWindowImpl->mpFrame)
    {
        SalFrame::SalPointerState aSalPointerState;

        aSalPointerState = mpWindowImpl->mpFrame->GetPointerState();
        if( ImplIsAntiparallel() )
        {
            // --- RTL --- (re-mirror mouse pos at this window)
            const OutputDevice *pOutDev = GetOutDev();
            pOutDev->ReMirror( aSalPointerState.maPos );
        }
        aState.maPos = ImplFrameToOutput( aSalPointerState.maPos );
        aState.mnState = aSalPointerState.mnState;
    }
    return aState;
}

bool Window::IsMouseOver()
{
    return ImplGetWinData()->mbMouseOver;
}

void Window::EnterWait()
{

    mpWindowImpl->mnWaitCount++;

    if ( mpWindowImpl->mnWaitCount == 1 )
    {
        // possibly immediately move pointer
        if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
            mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
    }
}

void Window::LeaveWait()
{

    if ( mpWindowImpl->mnWaitCount )
    {
        mpWindowImpl->mnWaitCount--;

        if ( !mpWindowImpl->mnWaitCount )
        {
            // possibly immediately move pointer
            if ( !mpWindowImpl->mpFrameData->mbInMouseMove && ImplTestMousePointerSet() )
                mpWindowImpl->mpFrame->SetPointer( ImplGetMousePointer() );
        }
    }
}

void Window::SetCursor( Cursor* pCursor )
{

    if ( mpWindowImpl->mpCursor != pCursor )
    {
        if ( mpWindowImpl->mpCursor )
            mpWindowImpl->mpCursor->ImplHide( true );
        mpWindowImpl->mpCursor = pCursor;
        if ( pCursor )
            pCursor->ImplShow();
    }
}

void Window::SetText( const OUString& rStr )
{
    if (rStr == mpWindowImpl->maText)
        return;

    OUString oldTitle( mpWindowImpl->maText );
    mpWindowImpl->maText = rStr;

    if ( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->SetText( rStr );
    else if ( mpWindowImpl->mbFrame )
        mpWindowImpl->mpFrame->SetTitle( rStr );

    ImplCallEventListeners( VCLEVENT_WINDOW_FRAMETITLECHANGED, &oldTitle );

    // #107247# needed for accessibility
    // The VCLEVENT_WINDOW_FRAMETITLECHANGED is (mis)used to notify accessible name changes.
    // Therefore a window, which is labeled by this window, must also notify an accessible
    // name change.
    if ( IsReallyVisible() )
    {
        Window* pWindow = GetAccessibleRelationLabelFor();
        if ( pWindow && pWindow != this )
            pWindow->ImplCallEventListeners( VCLEVENT_WINDOW_FRAMETITLECHANGED, &oldTitle );
    }

    StateChanged( STATE_CHANGE_TEXT );
}

OUString Window::GetText() const
{

    return mpWindowImpl->maText;
}

OUString Window::GetDisplayText() const
{

    return GetText();
}

const Wallpaper& Window::GetDisplayBackground() const
{
    // FIXME: fix issue 52349, need to fix this really in
    // all NWF enabled controls
    const ToolBox* pTB = dynamic_cast<const ToolBox*>(this);
    if( pTB )
    {
        if( IsNativeWidgetEnabled() )
            return pTB->ImplGetToolBoxPrivateData()->maDisplayBackground;
    }

    if( !IsBackground() )
    {
        if( mpWindowImpl->mpParent )
            return mpWindowImpl->mpParent->GetDisplayBackground();
    }

    const Wallpaper& rBack = GetBackground();
    if( ! rBack.IsBitmap() &&
        ! rBack.IsGradient() &&
        rBack.GetColor().GetColor() == COL_TRANSPARENT &&
        mpWindowImpl->mpParent )
            return mpWindowImpl->mpParent->GetDisplayBackground();
    return rBack;
}

const OUString& Window::GetHelpText() const
{

    OUString aStrHelpId( OStringToOUString( GetHelpId(), RTL_TEXTENCODING_UTF8 ) );
    bool bStrHelpId = !aStrHelpId.isEmpty();

    if ( !mpWindowImpl->maHelpText.getLength() && bStrHelpId )
    {
        if ( !IsDialog() && (mpWindowImpl->mnType != WINDOW_TABPAGE) && (mpWindowImpl->mnType != WINDOW_FLOATINGWINDOW) )
        {
            Help* pHelp = Application::GetHelp();
            if ( pHelp )
            {
                ((Window*)this)->mpWindowImpl->maHelpText = pHelp->GetHelpText( aStrHelpId, this );
                mpWindowImpl->mbHelpTextDynamic = false;
            }
        }
    }
    else if( mpWindowImpl->mbHelpTextDynamic && bStrHelpId )
    {
        static const char* pEnv = getenv( "HELP_DEBUG" );
        if( pEnv && *pEnv )
        {
            OUStringBuffer aTxt( 64+mpWindowImpl->maHelpText.getLength() );
            aTxt.append( mpWindowImpl->maHelpText );
            aTxt.appendAscii( "\n------------------\n" );
            aTxt.append( OUString( aStrHelpId ) );
            mpWindowImpl->maHelpText = aTxt.makeStringAndClear();
        }
        mpWindowImpl->mbHelpTextDynamic = false;
    }

    return mpWindowImpl->maHelpText;
}

Window* Window::FindWindow( const Point& rPos ) const
{

    Point aPos = OutputToScreenPixel( rPos );
    return ((Window*)this)->ImplFindWindow( aPos );
}

sal_uInt16 Window::GetChildCount() const
{

    sal_uInt16  nChildCount = 0;
    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        nChildCount++;
        pChild = pChild->mpWindowImpl->mpNext;
    }

    return nChildCount;
}

Window* Window::GetChild( sal_uInt16 nChild ) const
{

    sal_uInt16  nChildCount = 0;
    Window* pChild = mpWindowImpl->mpFirstChild;
    while ( pChild )
    {
        if ( nChild == nChildCount )
            return pChild;
        pChild = pChild->mpWindowImpl->mpNext;
        nChildCount++;
    }

    return NULL;
}

Window* Window::GetWindow( sal_uInt16 nType ) const
{

    switch ( nType )
    {
        case WINDOW_PARENT:
            return mpWindowImpl->mpRealParent;

        case WINDOW_FIRSTCHILD:
            return mpWindowImpl->mpFirstChild;

        case WINDOW_LASTCHILD:
            return mpWindowImpl->mpLastChild;

        case WINDOW_PREV:
            return mpWindowImpl->mpPrev;

        case WINDOW_NEXT:
            return mpWindowImpl->mpNext;

        case WINDOW_FIRSTOVERLAP:
            return mpWindowImpl->mpFirstOverlap;

        case WINDOW_LASTOVERLAP:
            return mpWindowImpl->mpLastOverlap;

        case WINDOW_OVERLAP:
            if ( ImplIsOverlapWindow() )
                return (Window*)this;
            else
                return mpWindowImpl->mpOverlapWindow;

        case WINDOW_PARENTOVERLAP:
            if ( ImplIsOverlapWindow() )
                return mpWindowImpl->mpOverlapWindow;
            else
                return mpWindowImpl->mpOverlapWindow->mpWindowImpl->mpOverlapWindow;

        case WINDOW_CLIENT:
            return ((Window*)this)->ImplGetWindow();

        case WINDOW_REALPARENT:
            return ImplGetParent();

        case WINDOW_FRAME:
            return mpWindowImpl->mpFrameWindow;

        case WINDOW_BORDER:
            if ( mpWindowImpl->mpBorderWindow )
                return mpWindowImpl->mpBorderWindow->GetWindow( WINDOW_BORDER );
            return (Window*)this;

        case WINDOW_FIRSTTOPWINDOWCHILD:
            return ImplGetWinData()->maTopWindowChildren.empty() ? NULL : *ImplGetWinData()->maTopWindowChildren.begin();

        case WINDOW_LASTTOPWINDOWCHILD:
            return ImplGetWinData()->maTopWindowChildren.empty() ? NULL : *ImplGetWinData()->maTopWindowChildren.rbegin();

        case WINDOW_PREVTOPWINDOWSIBLING:
        {
            if ( !mpWindowImpl->mpRealParent )
                return NULL;
            const ::std::list< Window* >& rTopWindows( mpWindowImpl->mpRealParent->ImplGetWinData()->maTopWindowChildren );
            ::std::list< Window* >::const_iterator myPos =
                ::std::find( rTopWindows.begin(), rTopWindows.end(), this );
            if ( myPos == rTopWindows.end() )
                return NULL;
            if ( myPos == rTopWindows.begin() )
                return NULL;
            return *--myPos;
        }

        case WINDOW_NEXTTOPWINDOWSIBLING:
        {
            if ( !mpWindowImpl->mpRealParent )
                return NULL;
            const ::std::list< Window* >& rTopWindows( mpWindowImpl->mpRealParent->ImplGetWinData()->maTopWindowChildren );
            ::std::list< Window* >::const_iterator myPos =
                ::std::find( rTopWindows.begin(), rTopWindows.end(), this );
            if ( ( myPos == rTopWindows.end() ) || ( ++myPos == rTopWindows.end() ) )
                return NULL;
            return *myPos;
        }

    }

    return NULL;
}

bool Window::IsChild( const Window* pWindow, bool bSystemWindow ) const
{

    do
    {
        if ( !bSystemWindow && pWindow->ImplIsOverlapWindow() )
            break;

        pWindow = pWindow->ImplGetParent();

        if ( pWindow == this )
            return true;
    }
    while ( pWindow );

    return false;
}

bool Window::IsWindowOrChild( const Window* pWindow, bool bSystemWindow ) const
{

    if ( this == pWindow )
        return true;
    return ImplIsChild( pWindow, bSystemWindow );
}

const SystemEnvData* Window::GetSystemData() const
{

    return mpWindowImpl->mpFrame ? mpWindowImpl->mpFrame->GetSystemData() : NULL;
}

::com::sun::star::uno::Any Window::GetSystemDataAny() const
{
    ::com::sun::star::uno::Any aRet;
    const SystemEnvData* pSysData = GetSystemData();
    if( pSysData )
    {
        ::com::sun::star::uno::Sequence< sal_Int8 > aSeq( (sal_Int8*)pSysData, pSysData->nSize );
        aRet <<= aSeq;
    }
    return aRet;
}

void Window::SetWindowPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xPeer, VCLXWindow* pVCLXWindow  )
{
    // be safe against re-entrance: first clear the old ref, then assign the new one
    // #133706# / 2006-03-30 / frank.schoenheit@sun.com
    mpWindowImpl->mxWindowPeer.clear();
    mpWindowImpl->mxWindowPeer = xPeer;

    mpWindowImpl->mpVCLXWindow = pVCLXWindow;
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > Window::GetComponentInterface( sal_Bool bCreate )
{
    if ( !mpWindowImpl->mxWindowPeer.is() && bCreate )
    {
        UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
        if ( pWrapper )
            mpWindowImpl->mxWindowPeer = pWrapper->GetWindowInterface( this, true );
    }
    return mpWindowImpl->mxWindowPeer;
}

void Window::SetComponentInterface( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xIFace )
{
    UnoWrapperBase* pWrapper = Application::GetUnoWrapper();
    DBG_ASSERT( pWrapper, "SetComponentInterface: No Wrapper!" );
    if ( pWrapper )
        pWrapper->SetWindowInterface( this, xIFace );
}

void Window::ImplCallDeactivateListeners( Window *pNew )
{
    // no deactivation if the newly activated window is my child
    if ( !pNew || !ImplIsChild( pNew ) )
    {
        ImplDelData aDogtag( this );
        ImplCallEventListeners( VCLEVENT_WINDOW_DEACTIVATE );
        if( aDogtag.IsDead() )
            return;

        // #100759#, avoid walking the wrong frame's hierarchy
        //           eg, undocked docking windows (ImplDockFloatWin)
        if ( ImplGetParent() && mpWindowImpl->mpFrameWindow == ImplGetParent()->mpWindowImpl->mpFrameWindow )
            ImplGetParent()->ImplCallDeactivateListeners( pNew );
    }
}

void Window::ImplCallActivateListeners( Window *pOld )
{
    // no activation if the old active window is my child
    if ( !pOld || !ImplIsChild( pOld ) )
    {
        ImplDelData aDogtag( this );
        ImplCallEventListeners( VCLEVENT_WINDOW_ACTIVATE, pOld );
        if( aDogtag.IsDead() )
            return;

        // #106298# revoke the change for 105369, because this change
        //          disabled the activate event for the parent,
        //          if the parent is a compound control
        //if( !GetParent() || !GetParent()->IsCompoundControl() )
        //{
            // #100759#, avoid walking the wrong frame's hierarchy
            //           eg, undocked docking windows (ImplDockFloatWin)
            // #104714#, revert the changes for 100759 because it has a side effect when pOld is a dialog
            //           additionally the gallery is not dockable anymore, so 100759 canot occur
        if ( ImplGetParent() ) /* && mpWindowImpl->mpFrameWindow == ImplGetParent()->mpWindowImpl->mpFrameWindow ) */
            ImplGetParent()->ImplCallActivateListeners( pOld );
        else if( (mpWindowImpl->mnStyle & WB_INTROWIN) == 0 )
        {
            // top level frame reached: store hint for DefModalDialogParent
            ImplGetSVData()->maWinData.mpActiveApplicationFrame = mpWindowImpl->mpFrameWindow;
        }
        //}
    }
}

bool Window::ImplStopDnd()
{
    bool bRet = false;
    if( mpWindowImpl->mpFrameData && mpWindowImpl->mpFrameData->mxDropTargetListener.is() )
    {
        bRet = true;
        mpWindowImpl->mpFrameData->mxDropTarget.clear();
        mpWindowImpl->mpFrameData->mxDragSource.clear();
        mpWindowImpl->mpFrameData->mxDropTargetListener.clear();
    }

    return bRet;
}

void Window::ImplStartDnd()
{
    GetDropTarget();
}

uno::Reference< XDropTarget > Window::GetDropTarget()
{

    if( ! mpWindowImpl->mxDNDListenerContainer.is() )
    {
        sal_Int8 nDefaultActions = 0;

        if( mpWindowImpl->mpFrameData )
        {
            if( ! mpWindowImpl->mpFrameData->mxDropTarget.is() )
            {
                // initialization is done in GetDragSource
                uno::Reference< XDragSource > xDragSource = GetDragSource();
            }

            if( mpWindowImpl->mpFrameData->mxDropTarget.is() )
            {
                nDefaultActions = mpWindowImpl->mpFrameData->mxDropTarget->getDefaultActions();

                if( ! mpWindowImpl->mpFrameData->mxDropTargetListener.is() )
                {
                    mpWindowImpl->mpFrameData->mxDropTargetListener = new DNDEventDispatcher( mpWindowImpl->mpFrameWindow );

                    try
                    {
                        mpWindowImpl->mpFrameData->mxDropTarget->addDropTargetListener( mpWindowImpl->mpFrameData->mxDropTargetListener );

                        // register also as drag gesture listener if directly supported by drag source
                        uno::Reference< XDragGestureRecognizer > xDragGestureRecognizer =
                            uno::Reference< XDragGestureRecognizer > (mpWindowImpl->mpFrameData->mxDragSource, UNO_QUERY);

                        if( xDragGestureRecognizer.is() )
                        {
                            xDragGestureRecognizer->addDragGestureListener(
                                uno::Reference< XDragGestureListener > (mpWindowImpl->mpFrameData->mxDropTargetListener, UNO_QUERY));
                        }
                        else
                            mpWindowImpl->mpFrameData->mbInternalDragGestureRecognizer = true;

                    }
                    catch (const RuntimeException&)
                    {
                        // release all instances
                        mpWindowImpl->mpFrameData->mxDropTarget.clear();
                        mpWindowImpl->mpFrameData->mxDragSource.clear();
                    }
                }
            }

        }

        mpWindowImpl->mxDNDListenerContainer = static_cast < XDropTarget * > ( new DNDListenerContainer( nDefaultActions ) );
    }

    // this object is located in the same process, so there will be no runtime exception
    return uno::Reference< XDropTarget > ( mpWindowImpl->mxDNDListenerContainer, UNO_QUERY );
}

uno::Reference< XDragSource > Window::GetDragSource()
{

#if HAVE_FEATURE_DESKTOP

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxDragSource.is() )
        {
            try
            {
                uno::Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );
                const SystemEnvData * pEnvData = GetSystemData();

                if( pEnvData )
                {
                    Sequence< Any > aDragSourceAL( 2 ), aDropTargetAL( 2 );
                    OUString aDragSourceSN, aDropTargetSN;
#if defined WNT
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.OleDragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.OleDropTarget";
                    aDragSourceAL[ 1 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->hWnd) ) );
                    aDropTargetAL[ 0 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->hWnd) ) );
#elif defined MACOSX
            /* FIXME: Mac OS X specific dnd interface does not exist! *
             * Using Windows based dnd as a temporary solution        */
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.OleDragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.OleDropTarget";
                    aDragSourceAL[ 1 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->mpNSView) ) );
                    aDropTargetAL[ 0 ] = makeAny( static_cast<sal_uInt64>( reinterpret_cast<sal_IntPtr>(pEnvData->mpNSView) ) );
#elif HAVE_FEATURE_X11
                    aDragSourceSN = "com.sun.star.datatransfer.dnd.X11DragSource";
                    aDropTargetSN = "com.sun.star.datatransfer.dnd.X11DropTarget";

                    aDragSourceAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                    aDropTargetAL[ 0 ] = makeAny( Application::GetDisplayConnection() );
                    aDropTargetAL[ 1 ] = makeAny( (sal_Size)(pEnvData->aShellWindow) );
#endif
                    if( !aDragSourceSN.isEmpty() )
                        mpWindowImpl->mpFrameData->mxDragSource.set(
                            xContext->getServiceManager()->createInstanceWithArgumentsAndContext( aDragSourceSN, aDragSourceAL, xContext ),
                            UNO_QUERY );

                    if( !aDropTargetSN.isEmpty() )
                        mpWindowImpl->mpFrameData->mxDropTarget.set(
                           xContext->getServiceManager()->createInstanceWithArgumentsAndContext( aDropTargetSN, aDropTargetAL, xContext ),
                           UNO_QUERY );
                }
            }

            // createInstance can throw any exception
            catch (const Exception&)
            {
                // release all instances
                mpWindowImpl->mpFrameData->mxDropTarget.clear();
                mpWindowImpl->mpFrameData->mxDragSource.clear();
            }
        }

        return mpWindowImpl->mpFrameData->mxDragSource;
    }
#endif
    return uno::Reference< XDragSource > ();
}

uno::Reference< XDragGestureRecognizer > Window::GetDragGestureRecognizer()
{
    return uno::Reference< XDragGestureRecognizer > ( GetDropTarget(), UNO_QUERY );
}

uno::Reference< XClipboard > Window::GetClipboard()
{

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxClipboard.is() )
        {
            try
            {
                mpWindowImpl->mpFrameData->mxClipboard
                    = css::datatransfer::clipboard::SystemClipboard::create(
                        comphelper::getProcessComponentContext());
            }
            catch (css::uno::DeploymentException & e)
            {
                SAL_WARN(
                    "vcl.window",
                    "ignoring DeploymentException \"" << e.Message << "\"");
            }
        }

        return mpWindowImpl->mpFrameData->mxClipboard;
    }

    return static_cast < XClipboard * > (0);
}

uno::Reference< XClipboard > Window::GetPrimarySelection()
{

    if( mpWindowImpl->mpFrameData )
    {
        if( ! mpWindowImpl->mpFrameData->mxSelection.is() )
        {
            try
            {
                uno::Reference< XComponentContext > xContext( comphelper::getProcessComponentContext() );

#if HAVE_FEATURE_X11
                // A hack, making the primary selection available as an instance
                // of the SystemClipboard service on X11:
                css::uno::Sequence<css::uno::Any> args(1);
                args[0] <<= OUString("PRIMARY");
                mpWindowImpl->mpFrameData->mxSelection.set(
                    (xContext->getServiceManager()->
                     createInstanceWithArgumentsAndContext(
                         "com.sun.star.datatransfer.clipboard.SystemClipboard",
                         args, xContext)),
                    css::uno::UNO_QUERY_THROW);
#       else
                static uno::Reference< XClipboard > s_xSelection(
                    xContext->getServiceManager()->createInstanceWithContext( "com.sun.star.datatransfer.clipboard.GenericClipboard", xContext ), UNO_QUERY );

                mpWindowImpl->mpFrameData->mxSelection = s_xSelection;
#       endif
            }
            catch (css::uno::RuntimeException & e)
            {
                SAL_WARN(
                    "vcl.window",
                    "ignoring RuntimeException \"" << e.Message << "\"");
            }
        }

        return mpWindowImpl->mpFrameData->mxSelection;
    }

    return static_cast < XClipboard * > (0);
}

void Window::RecordLayoutData( vcl::ControlLayoutData* pLayout, const Rectangle& rRect )
{
    assert(mpOutDevData);
    mpOutDevData->mpRecordLayout = pLayout;
    mpOutDevData->maRecordRect = rRect;
    Paint( rRect );
    mpOutDevData->mpRecordLayout = NULL;
}

void Window::DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, bool bChecked, bool bDrawBorder, bool bDrawExtBorderOnly )
{
    DrawSelectionBackground( rRect, highlight, bChecked, bDrawBorder, bDrawExtBorderOnly, 0, NULL, NULL );
}

void Window::DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, bool bChecked, bool bDrawBorder, bool bDrawExtBorderOnly, Color* pSelectionTextColor )
{
    DrawSelectionBackground( rRect, highlight, bChecked, bDrawBorder, bDrawExtBorderOnly, 0, pSelectionTextColor, NULL );
}

void Window::DrawSelectionBackground( const Rectangle& rRect,
                                      sal_uInt16 highlight,
                                      bool bChecked,
                                      bool bDrawBorder,
                                      bool bDrawExtBorderOnly,
                                      long nCornerRadius,
                                      Color* pSelectionTextColor,
                                      Color* pPaintColor
                                      )
{
    if( rRect.IsEmpty() )
        return;

    bool bRoundEdges = nCornerRadius > 0;

    const StyleSettings& rStyles = GetSettings().GetStyleSettings();

    // colors used for item highlighting
    Color aSelectionBorderCol( pPaintColor ? *pPaintColor : rStyles.GetHighlightColor() );
    Color aSelectionFillCol( aSelectionBorderCol );

    bool bDark = rStyles.GetFaceColor().IsDark();
    bool bBright = ( rStyles.GetFaceColor() == Color( COL_WHITE ) );

    int c1 = aSelectionBorderCol.GetLuminance();
    int c2 = GetDisplayBackground().GetColor().GetLuminance();

    if( !bDark && !bBright && abs( c2-c1 ) < (pPaintColor ? 40 : 75) )
    {
        // constrast too low
        sal_uInt16 h,s,b;
        aSelectionFillCol.RGBtoHSB( h, s, b );
        if( b > 50 )    b -= 40;
        else            b += 40;
        aSelectionFillCol.SetColor( Color::HSBtoRGB( h, s, b ) );
        aSelectionBorderCol = aSelectionFillCol;
    }

    if( bRoundEdges )
    {
        if( aSelectionBorderCol.IsDark() )
            aSelectionBorderCol.IncreaseLuminance( 128 );
        else
            aSelectionBorderCol.DecreaseLuminance( 128 );
    }

    Rectangle aRect( rRect );
    if( bDrawExtBorderOnly )
    {
        --aRect.Left();
        --aRect.Top();
        ++aRect.Right();
        ++aRect.Bottom();
    }
    Color oldFillCol = GetFillColor();
    Color oldLineCol = GetLineColor();

    if( bDrawBorder )
        SetLineColor( bDark ? Color(COL_WHITE) : ( bBright ? Color(COL_BLACK) : aSelectionBorderCol ) );
    else
        SetLineColor();

    sal_uInt16 nPercent = 0;
    if( !highlight )
    {
        if( bDark )
            aSelectionFillCol = COL_BLACK;
        else
            nPercent = 80;  // just checked (light)
    }
    else
    {
        if( bChecked && highlight == 2 )
        {
            if( bDark )
                aSelectionFillCol = COL_LIGHTGRAY;
            else if ( bBright )
            {
                aSelectionFillCol = COL_BLACK;
                SetLineColor( COL_BLACK );
                nPercent = 0;
            }
            else
                nPercent = bRoundEdges ? 40 : 20;          // selected, pressed or checked ( very dark )
        }
        else if( bChecked || highlight == 1 )
        {
            if( bDark )
                aSelectionFillCol = COL_GRAY;
            else if ( bBright )
            {
                aSelectionFillCol = COL_BLACK;
                SetLineColor( COL_BLACK );
                nPercent = 0;
            }
            else
                nPercent = bRoundEdges ? 60 : 35;          // selected, pressed or checked ( very dark )
        }
        else
        {
            if( bDark )
                aSelectionFillCol = COL_LIGHTGRAY;
            else if ( bBright )
            {
                aSelectionFillCol = COL_BLACK;
                SetLineColor( COL_BLACK );
                if( highlight == 3 )
                    nPercent = 80;
                else
                    nPercent = 0;
            }
            else
                nPercent = 70;          // selected ( dark )
        }
    }

    if( bDark && bDrawExtBorderOnly )
    {
        SetFillColor();
        if( pSelectionTextColor )
            *pSelectionTextColor = rStyles.GetHighlightTextColor();
    }
    else
    {
        SetFillColor( aSelectionFillCol );
        if( pSelectionTextColor )
        {
            Color aTextColor = IsControlBackground() ? GetControlForeground() : rStyles.GetButtonTextColor();
            Color aHLTextColor = rStyles.GetHighlightTextColor();
            int nTextDiff = abs(aSelectionFillCol.GetLuminance() - aTextColor.GetLuminance());
            int nHLDiff = abs(aSelectionFillCol.GetLuminance() - aHLTextColor.GetLuminance());
            *pSelectionTextColor = (nHLDiff >= nTextDiff) ? aHLTextColor : aTextColor;
        }
    }

    if( bDark )
    {
        DrawRect( aRect );
    }
    else
    {
        if( bRoundEdges )
        {
            Polygon aPoly( aRect, nCornerRadius, nCornerRadius );
            PolyPolygon aPolyPoly( aPoly );
            DrawTransparent( aPolyPoly, nPercent );
        }
        else
        {
            Polygon aPoly( aRect );
            PolyPolygon aPolyPoly( aPoly );
            DrawTransparent( aPolyPoly, nPercent );
        }
    }

    SetFillColor( oldFillCol );
    SetLineColor( oldLineCol );
}

// controls should return the window that gets the
// focus by default, so keyevents can be sent to that window directly
Window* Window::GetPreferredKeyInputWindow()
{
    return this;
}

bool Window::IsScrollable() const
{
    // check for scrollbars
    Window *pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        if( pChild->GetType() == WINDOW_SCROLLBAR )
            return true;
        else
            pChild = pChild->mpWindowImpl->mpNext;
    }
    return false;
}

void Window::ImplMirrorFramePos( Point &pt ) const
{
    pt.X() = mpWindowImpl->mpFrame->maGeometry.nWidth-1-pt.X();
}

// frame based modal counter (dialogs are not modal to the whole application anymore)
bool Window::IsInModalMode() const
{
    return (mpWindowImpl->mpFrameWindow->mpWindowImpl->mpFrameData->mnModalMode != 0);
}

bool Window::IsInModalNonRefMode() const
{
    if(mpWindowImpl->mnStyle & WB_REFMODE)
        return false;

    return IsInModalMode();
}

void Window::ImplIncModalCount()
{
    Window* pFrameWindow = mpWindowImpl->mpFrameWindow;
    Window* pParent = pFrameWindow;
    while( pFrameWindow )
    {
        pFrameWindow->mpWindowImpl->mpFrameData->mnModalMode++;
        while( pParent && pParent->mpWindowImpl->mpFrameWindow == pFrameWindow )
        {
            pParent = pParent->GetParent();
        }
        pFrameWindow = pParent ? pParent->mpWindowImpl->mpFrameWindow : NULL;
    }
}
void Window::ImplDecModalCount()
{
    Window* pFrameWindow = mpWindowImpl->mpFrameWindow;
    Window* pParent = pFrameWindow;
    while( pFrameWindow )
    {
        pFrameWindow->mpWindowImpl->mpFrameData->mnModalMode--;
        while( pParent && pParent->mpWindowImpl->mpFrameWindow == pFrameWindow )
        {
            pParent = pParent->GetParent();
        }
        pFrameWindow = pParent ? pParent->mpWindowImpl->mpFrameWindow : NULL;
    }
}

void Window::ImplIsInTaskPaneList( bool mbIsInTaskList )
{
    mpWindowImpl->mbIsInTaskPaneList = mbIsInTaskList;
}

void Window::ImplNotifyIconifiedState( bool bIconified )
{
    mpWindowImpl->mpFrameWindow->ImplCallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
    // #109206# notify client window as well to have toolkit topwindow listeners notified
    if( mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow && mpWindowImpl->mpFrameWindow != mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow )
        mpWindowImpl->mpFrameWindow->mpWindowImpl->mpClientWindow->ImplCallEventListeners( bIconified ? VCLEVENT_WINDOW_MINIMIZE : VCLEVENT_WINDOW_NORMALIZE );
}

bool Window::HasActiveChildFrame()
{
    bool bRet = false;
    Window *pFrameWin = ImplGetSVData()->maWinData.mpFirstFrame;
    while( pFrameWin )
    {
        if( pFrameWin != mpWindowImpl->mpFrameWindow )
        {
            bool bDecorated = false;
            Window *pChildFrame = pFrameWin->ImplGetWindow();
            // #i15285# unfortunately WB_MOVEABLE is the same as WB_TABSTOP which can
            // be removed for ToolBoxes to influence the keyboard accessibility
            // thus WB_MOVEABLE is no indicator for decoration anymore
            // but FloatingWindows carry this information in their TitleType...
            // TODO: avoid duplicate WinBits !!!
            if( pChildFrame && pChildFrame->ImplIsFloatingWindow() )
                bDecorated = ((FloatingWindow*) pChildFrame)->GetTitleType() != FLOATWIN_TITLE_NONE;
            if( bDecorated || (pFrameWin->mpWindowImpl->mnStyle & (WB_MOVEABLE | WB_SIZEABLE) ) )
                if( pChildFrame && pChildFrame->IsVisible() && pChildFrame->IsActive() )
                {
                    if( ImplIsChild( pChildFrame, true ) )
                    {
                        bRet = true;
                        break;
                    }
                }
        }
        pFrameWin = pFrameWin->mpWindowImpl->mpFrameData->mpNextFrame;
    }
    return bRet;
}

LanguageType Window::GetInputLanguage() const
{
    return mpWindowImpl->mpFrame->GetInputLanguage();
}

void Window::EnableNativeWidget( bool bEnable )
{
    static const char* pNoNWF = getenv( "SAL_NO_NWF" );
    if( pNoNWF && *pNoNWF )
        bEnable = false;

    if( bEnable != ImplGetWinData()->mbEnableNativeWidget )
    {
        ImplGetWinData()->mbEnableNativeWidget = bEnable;

        // send datachanged event to allow for internal changes required for NWF
        // like clipmode, transparency, etc.
        DataChangedEvent aDCEvt( DATACHANGED_SETTINGS, mxSettings.get(), SETTINGS_STYLE );
        DataChanged( aDCEvt );

        // sometimes the borderwindow is queried, so keep it in sync
        if( mpWindowImpl->mpBorderWindow )
            mpWindowImpl->mpBorderWindow->ImplGetWinData()->mbEnableNativeWidget = bEnable;
    }

    // push down, useful for compound controls
    Window *pChild = mpWindowImpl->mpFirstChild;
    while( pChild )
    {
        pChild->EnableNativeWidget( bEnable );
        pChild = pChild->mpWindowImpl->mpNext;
    }
}

bool Window::IsNativeWidgetEnabled() const
{
    return ImplGetWinData()->mbEnableNativeWidget;
}

#ifdef WNT // see #140456#
#include <win/salframe.h>
#endif

uno::Reference< rendering::XCanvas > Window::ImplGetCanvas( const Size& rFullscreenSize,
                                                       bool        bFullscreen,
                                                       bool        bSpriteCanvas ) const
{
    // try to retrieve hard reference from weak member
    uno::Reference< rendering::XCanvas > xCanvas( mpWindowImpl->mxCanvas );

    // canvas still valid? Then we're done.
    if( xCanvas.is() )
        return xCanvas;

    Sequence< Any > aArg(6);

    // Feed any with operating system's window handle

    // common: first any is VCL pointer to window (for VCL canvas)
    aArg[ 0 ] = makeAny( reinterpret_cast<sal_Int64>(this) );

    // TODO(Q1): Make GetSystemData method virtual

    // check whether we're a SysChild: have to fetch system data
    // directly from SystemChildWindow, because the GetSystemData
    // method is unfortunately not virtual
    const SystemChildWindow* pSysChild = dynamic_cast< const SystemChildWindow* >( this );
    if( pSysChild )
    {
        aArg[ 1 ] = pSysChild->GetSystemDataAny();
        aArg[ 5 ] = pSysChild->GetSystemGfxDataAny();
    }
    else
    {
        aArg[ 1 ] = GetSystemDataAny();
        aArg[ 5 ] = GetSystemGfxDataAny();
    }

    if( bFullscreen )
        aArg[ 2 ] = makeAny( ::com::sun::star::awt::Rectangle( 0, 0,
                                                               rFullscreenSize.Width(),
                                                               rFullscreenSize.Height() ) );
    else
        aArg[ 2 ] = makeAny( ::com::sun::star::awt::Rectangle( mnOutOffX, mnOutOffY, mnOutWidth, mnOutHeight ) );

    aArg[ 3 ] = makeAny( mpWindowImpl->mbAlwaysOnTop ? sal_True : sal_False );
    aArg[ 4 ] = makeAny( uno::Reference< awt::XWindow >(
                             const_cast<Window*>(this)->GetComponentInterface(),
                             uno::UNO_QUERY ));

    uno::Reference< XComponentContext > xContext = comphelper::getProcessComponentContext();

    // Create canvas instance with window handle

    static ::vcl::DeleteUnoReferenceOnDeinit<lang::XMultiComponentFactory> xStaticCanvasFactory(
        rendering::CanvasFactory::create( xContext ) );
    uno::Reference<lang::XMultiComponentFactory> xCanvasFactory(xStaticCanvasFactory.get());

    if(xCanvasFactory.is())
    {
#ifdef WNT
        // see #140456# - if we're running on a multiscreen setup,
        // request special, multi-screen safe sprite canvas
        // implementation (not DX5 canvas, as it cannot cope with
        // surfaces spanning multiple displays). Note: canvas
        // (without sprite) stays the same)
        const sal_uInt32 nDisplay = static_cast< WinSalFrame* >( mpWindowImpl->mpFrame )->mnDisplay;
        if( (nDisplay >= Application::GetScreenCount()) )
        {
            xCanvas.set( xCanvasFactory->createInstanceWithArgumentsAndContext(
                                 bSpriteCanvas ?
                                 OUString( "com.sun.star.rendering.SpriteCanvas.MultiScreen" ) :
                                 OUString( "com.sun.star.rendering.Canvas.MultiScreen" ),
                                 aArg,
                                 xContext ),
                             UNO_QUERY );

        }
        else
        {
#endif
            xCanvas.set( xCanvasFactory->createInstanceWithArgumentsAndContext(
                             bSpriteCanvas ?
                             OUString( "com.sun.star.rendering.SpriteCanvas" ) :
                             OUString( "com.sun.star.rendering.Canvas" ),
                             aArg,
                             xContext ),
                         UNO_QUERY );

#ifdef WNT
        }
#endif
        mpWindowImpl->mxCanvas = xCanvas;
    }

    // no factory??? Empty reference, then.
    return xCanvas;
}

uno::Reference< rendering::XCanvas > Window::GetCanvas() const
{
    return ImplGetCanvas( Size(), false, false );
}

uno::Reference< rendering::XSpriteCanvas > Window::GetSpriteCanvas() const
{
    uno::Reference< rendering::XSpriteCanvas > xSpriteCanvas(
        ImplGetCanvas( Size(), false, true ), uno::UNO_QUERY );
    return xSpriteCanvas;
}

void Window::ImplPaintToDevice( OutputDevice* i_pTargetOutDev, const Point& i_rPos )
{
    bool bRVisible = mpWindowImpl->mbReallyVisible;
    mpWindowImpl->mbReallyVisible = mpWindowImpl->mbVisible;
    bool bDevOutput = mbDevOutput;
    mbDevOutput = true;

    const OutputDevice *pOutDev = GetOutDev();
    long nOldDPIX = pOutDev->GetDPIX();
    long nOldDPIY = pOutDev->GetDPIY();
    mnDPIX = i_pTargetOutDev->GetDPIX();
    mnDPIY = i_pTargetOutDev->GetDPIY();
    bool bOutput = IsOutputEnabled();
    EnableOutput();

    DBG_ASSERT( GetMapMode().GetMapUnit() == MAP_PIXEL, "MapMode must be PIXEL based" );
    if ( GetMapMode().GetMapUnit() != MAP_PIXEL )
        return;

    // preserve graphicsstate
    Push();
    Region aClipRegion( GetClipRegion() );
    SetClipRegion();

    GDIMetaFile* pOldMtf = GetConnectMetaFile();
    GDIMetaFile aMtf;
    SetConnectMetaFile( &aMtf );

    // put a push action to metafile
    Push();
    // copy graphics state to metafile
    Font aCopyFont = GetFont();
    if( nOldDPIX != mnDPIX || nOldDPIY != mnDPIY )
    {
        aCopyFont.SetHeight( aCopyFont.GetHeight() * mnDPIY / nOldDPIY );
        aCopyFont.SetWidth( aCopyFont.GetWidth() * mnDPIX / nOldDPIX );
    }
    SetFont( aCopyFont );
    SetTextColor( GetTextColor() );
    if( IsLineColor() )
        SetLineColor( GetLineColor() );
    else
        SetLineColor();
    if( IsFillColor() )
        SetFillColor( GetFillColor() );
    else
        SetFillColor();
    if( IsTextLineColor() )
        SetTextLineColor( GetTextLineColor() );
    else
        SetTextLineColor();
    if( IsOverlineColor() )
        SetOverlineColor( GetOverlineColor() );
    else
        SetOverlineColor();
    if( IsTextFillColor() )
        SetTextFillColor( GetTextFillColor() );
    else
        SetTextFillColor();
    SetTextAlign( GetTextAlign() );
    SetRasterOp( GetRasterOp() );
    if( IsRefPoint() )
        SetRefPoint( GetRefPoint() );
    else
        SetRefPoint();
    SetLayoutMode( GetLayoutMode() );
    SetDigitLanguage( GetDigitLanguage() );
    Rectangle aPaintRect( Point( 0, 0 ), GetOutputSizePixel() );
    aClipRegion.Intersect( aPaintRect );
    SetClipRegion( aClipRegion );

    // do the actual paint

    // background
    if( ! IsPaintTransparent() && IsBackground() && ! (GetParentClipMode() & PARENTCLIPMODE_NOCLIP ) )
        Erase();
    // foreground
    Paint( aPaintRect );
    // put a pop action to metafile
    Pop();

    SetConnectMetaFile( pOldMtf );
    EnableOutput( bOutput );
    mpWindowImpl->mbReallyVisible = bRVisible;

    // paint metafile to VDev
    VirtualDevice* pMaskedDevice = new VirtualDevice( *i_pTargetOutDev, 0, 0 );
    pMaskedDevice->SetOutputSizePixel( GetOutputSizePixel() );
    pMaskedDevice->EnableRTL( IsRTLEnabled() );
    aMtf.WindStart();
    aMtf.Play( pMaskedDevice );
    BitmapEx aBmpEx( pMaskedDevice->GetBitmapEx( Point( 0, 0 ), pMaskedDevice->GetOutputSizePixel() ) );
    i_pTargetOutDev->DrawBitmapEx( i_rPos, aBmpEx );
    // get rid of virtual device now so they don't pile up during recursive calls
    delete pMaskedDevice, pMaskedDevice = NULL;

    for( Window* pChild = mpWindowImpl->mpFirstChild; pChild; pChild = pChild->mpWindowImpl->mpNext )
    {
        if( pChild->mpWindowImpl->mpFrame == mpWindowImpl->mpFrame && pChild->IsVisible() )
        {
            long nDeltaX = pChild->mnOutOffX - mnOutOffX;

            if( pOutDev->HasMirroredGraphics() )
                nDeltaX = mnOutWidth - nDeltaX - pChild->mnOutWidth;
            long nDeltaY = pChild->GetOutOffYPixel() - GetOutOffYPixel();
            Point aPos( i_rPos );
            Point aDelta( nDeltaX, nDeltaY );
            aPos += aDelta;
            pChild->ImplPaintToDevice( i_pTargetOutDev, aPos );
        }
    }

    // restore graphics state
    Pop();

    EnableOutput( bOutput );
    mpWindowImpl->mbReallyVisible = bRVisible;
    mbDevOutput = bDevOutput;
    mnDPIX = nOldDPIX;
    mnDPIY = nOldDPIY;
}

void Window::PaintToDevice( OutputDevice* pDev, const Point& rPos, const Size& /*rSize*/ )
{
    // FIXME: scaling: currently this is for pixel copying only

    DBG_ASSERT( ! pDev->HasMirroredGraphics(), "PaintToDevice to mirroring graphics" );
    DBG_ASSERT( ! pDev->IsRTLEnabled(), "PaintToDevice to mirroring device" );

    Window* pRealParent = NULL;
    if( ! mpWindowImpl->mbVisible )
    {
        Window* pTempParent = ImplGetDefaultWindow();
        if( pTempParent )
            pTempParent->EnableChildTransparentMode();
        pRealParent = GetParent();
        SetParent( pTempParent );
        // trigger correct visibility flags for children
        Show();
        Hide();
    }

    bool bVisible = mpWindowImpl->mbVisible;
    mpWindowImpl->mbVisible = true;

    if( mpWindowImpl->mpBorderWindow )
        mpWindowImpl->mpBorderWindow->ImplPaintToDevice( pDev, rPos );
    else
        ImplPaintToDevice( pDev, rPos );

    mpWindowImpl->mbVisible = bVisible;

    if( pRealParent )
        SetParent( pRealParent );
}

OUString Window::GetSurroundingText() const
{
  return OUString();
}

Selection Window::GetSurroundingTextSelection() const
{
  return Selection( 0, 0 );
}

bool Window::UsePolyPolygonForComplexGradient()
{
    if ( meRasterOp != ROP_OVERPAINT )
        return true;

    return false;
}

void Window::DrawGradientWallpaper( long nX, long nY,
                                    long nWidth, long nHeight,
                                    const Wallpaper& rWallpaper )
{
    Rectangle       aBound;
    GDIMetaFile*    pOldMetaFile = mpMetaFile;
    const bool      bOldMap = mbMap;
    bool            bNeedGradient = true;

    aBound = Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) );

    mpMetaFile = NULL;
    EnableMapMode( false );
    Push( PUSH_CLIPREGION );
    IntersectClipRegion( Rectangle( Point( nX, nY ), Size( nWidth, nHeight ) ) );

    if( rWallpaper.GetStyle() == WALLPAPER_APPLICATIONGRADIENT )
    {
        // limit gradient to useful size, so that it still can be noticed
        // in maximized windows
        long gradientWidth = GetDesktopRectPixel().GetSize().Width();
        if( gradientWidth > 1024 )
            gradientWidth = 1024;
        if( mnOutOffX+nWidth > gradientWidth )
            DrawColorWallpaper(  nX, nY, nWidth, nHeight, rWallpaper.GetGradient().GetEndColor() );
        if( mnOutOffX > gradientWidth )
            bNeedGradient = false;
        else
            aBound = Rectangle( Point( -mnOutOffX, nY ), Size( gradientWidth, nHeight ) );
    }

    if( bNeedGradient )
        DrawGradient( aBound, rWallpaper.GetGradient() );

    Pop();
    EnableMapMode( bOldMap );
    mpMetaFile = pOldMetaFile;
}

void Window::Erase()
{
    if ( !IsDeviceOutputNecessary() || ImplIsRecordLayout() )
        return;

    bool bNativeOK = false;

    ControlPart aCtrlPart = ImplGetWindowImpl()->mnNativeBackground;
    if( aCtrlPart != 0 && ! IsControlBackground() )
    {
        Rectangle           aCtrlRegion( Point(), GetOutputSizePixel() );
        ControlState        nState = 0;

        if( IsEnabled() )
            nState |= CTRL_STATE_ENABLED;

        bNativeOK = DrawNativeControl( CTRL_WINDOW_BACKGROUND, aCtrlPart, aCtrlRegion,
                                       nState, ImplControlValue(), OUString() );
    }

    if ( mbBackground && ! bNativeOK )
    {
        RasterOp eRasterOp = GetRasterOp();
        if ( eRasterOp != ROP_OVERPAINT )
            SetRasterOp( ROP_OVERPAINT );
        DrawWallpaper( 0, 0, mnOutWidth, mnOutHeight, maBackground );
        if ( eRasterOp != ROP_OVERPAINT )
            SetRasterOp( eRasterOp );
    }

    if( mpAlphaVDev )
        mpAlphaVDev->Erase();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
