/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 This file has been autogenerated by update_pch.sh . It is possible to edit it
 manually (such as when an include file has been moved/renamed/removed. All such
 manual changes will be rewritten by the next run of update_pch.sh (which presumably
 also fixes all possible problems, so it's usually better to use it).
*/

#include "basegfx/matrix/b2dhommatrix.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/polygon/b2dpolygontools.hxx"
#include "basegfx/polygon/b2dpolypolygon.hxx"
#include "com/sun/star/accessibility/AccessibleRole.hpp"
#include "com/sun/star/accessibility/MSAAService.hpp"
#include "com/sun/star/accessibility/XAccessible.hpp"
#include "com/sun/star/awt/Size.hpp"
#include "com/sun/star/awt/XDisplayConnection.hpp"
#include "com/sun/star/awt/XToolkit.hpp"
#include "com/sun/star/awt/XTopWindow.hpp"
#include "com/sun/star/awt/XWindow.hpp"
#include "com/sun/star/awt/XWindowPeer.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/PropertyValues.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/configuration/theDefaultProvider.hpp"
#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/datatransfer/XTransferable.hpp"
#include "com/sun/star/datatransfer/clipboard/SystemClipboard.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboard.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardEx.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardListener.hpp"
#include "com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp"
#include "com/sun/star/datatransfer/clipboard/XSystemClipboard.hpp"
#include "com/sun/star/datatransfer/dnd/DNDConstants.hpp"
#include "com/sun/star/datatransfer/dnd/XDragSource.hpp"
#include "com/sun/star/datatransfer/dnd/XDropTarget.hpp"
#include "com/sun/star/frame/Desktop.hpp"
#include "com/sun/star/graphic/GraphicProvider.hpp"
#include "com/sun/star/graphic/XGraphicProvider.hpp"
#include "com/sun/star/i18n/TransliterationModules.hpp"
#include "com/sun/star/i18n/WordType.hpp"
#include "com/sun/star/i18n/XBreakIterator.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/io/XSeekable.hpp"
#include "com/sun/star/lang/DisposedException.hpp"
#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XServiceName.hpp"
#include "com/sun/star/lang/XSingleServiceFactory.hpp"
#include "com/sun/star/linguistic2/LinguServiceManager.hpp"
#include "com/sun/star/packages/zip/ZipFileAccess.hpp"
#include "com/sun/star/rendering/CanvasFactory.hpp"
#include "com/sun/star/rendering/XCanvas.hpp"
#include "com/sun/star/rendering/XSpriteCanvas.hpp"
#include "com/sun/star/ui/dialogs/ExecutableDialogResults.hpp"
#include "com/sun/star/ui/dialogs/FilePicker.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/XFilterManager.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.h"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XNamingService.hpp"
#include "com/sun/star/view/DuplexMode.hpp"
#include "comphelper/processfactory.hxx"
#include "comphelper/solarmutex.hxx"
#include "cppuhelper/compbase1.hxx"
#include "cppuhelper/compbase2.hxx"
#include "cppuhelper/implbase1.hxx"
#include "i18nlangtag/languagetag.hxx"
#include "i18nlangtag/mslangid.hxx"
#include "i18nutil/unicode.hxx"
#include "officecfg/Office/Common.hxx"
#include "osl/diagnose.h"
#include "osl/file.h"
#include "osl/file.hxx"
#include "osl/module.h"
#include "osl/module.hxx"
#include "osl/mutex.hxx"
#include "osl/process.h"
#include "osl/thread.h"
#include "rtl/bootstrap.hxx"
#include "rtl/instance.hxx"
#include "rtl/logfile.hxx"
#include "rtl/math.hxx"
#include "rtl/process.h"
#include "rtl/strbuf.hxx"
#include "rtl/string.hxx"
#include "rtl/tencinfo.h"
#include "rtl/textcvt.h"
#include "rtl/ustrbuf.hxx"
#include "sal/alloca.h"
#include "sal/config.h"
#include "stdio.h"
#include "string.h"
#include "tools/debug.hxx"
#include "tools/diagnose_ex.h"
#include "tools/poly.hxx"
#include "tools/rc.h"
#include "tools/resmgr.hxx"
#include "tools/stream.hxx"
#include "tools/time.hxx"
#include "tools/urlobj.hxx"
#include "tools/vcompat.hxx"
#include "uno/current_context.hxx"
#include "unotools/collatorwrapper.hxx"
#include "unotools/confignode.hxx"
#include "unotools/fontcfg.hxx"
#include "unotools/fontcvt.hxx"
#include "unotools/localedatawrapper.hxx"
#include "unotools/streamwrap.hxx"
#include "unotools/syslocaleoptions.hxx"
#include "unotools/transliterationwrapper.hxx"
#include <algorithm>
#include <assert.h>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/polygon/b2dlinegeometry.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/range/b2irectangle.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/vector/b2dsize.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/vector/b2isize.hxx>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <cmath>
#include <com/sun/star/accessibility/AccessibleEventObject.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontFamily.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/MouseButton.hpp>
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/awt/XExtendedToolkit.hpp>
#include <com/sun/star/awt/XGraphics.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSource.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XModuleManager2.hpp>
#include <com/sun/star/frame/XSessionManagerClient.hpp>
#include <com/sun/star/frame/XSessionManagerListener2.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/geometry/IntegerPoint2D.hpp>
#include <com/sun/star/geometry/IntegerRectangle2D.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>
#include <com/sun/star/geometry/RealBezierSegment2D.hpp>
#include <com/sun/star/geometry/RealPoint2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/Primitive2DTools.hpp>
#include <com/sun/star/graphic/SvgTools.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicRenderer.hpp>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <com/sun/star/i18n/CharacterClassification.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/InputSequenceCheckMode.hpp>
#include <com/sun/star/i18n/InputSequenceChecker.hpp>
#include <com/sun/star/i18n/KCharacterType.hpp>
#include <com/sun/star/i18n/ScriptDirection.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/i18n/XCollator.hpp>
#include <com/sun/star/io/XActiveDataSource.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <com/sun/star/rendering/CanvasFactory.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>
#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/rendering/MtfRenderer.hpp>
#include <com/sun/star/rendering/RenderingIntent.hpp>
#include <com/sun/star/rendering/XBitmap.hpp>
#include <com/sun/star/rendering/XBitmapCanvas.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <com/sun/star/rendering/XGraphicDevice.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/rendering/XIntegerReadOnlyBitmap.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/svg/XSVGWriter.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/uno/XWeak.hpp>
#include <com/sun/star/util/Endianness.hpp>
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchOptions.hpp>
#include <com/sun/star/util/URL.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <com/sun/star/util/thePathSettings.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/seqstream.hxx>
#include <comphelper/string.hxx>
#include <config_eot.h>
#include <config_features.h>
#include <config_folders.h>
#include <config_graphite.h>
#include <config_libraries.h>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/weak.hxx>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>
#include <fcntl.h>
#include <functional>
#include <i18nlangtag/lang.h>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <iomanip>
#include <iostream>
#include <jerror.h>
#include <jpeglib.h>
#include <lcms2.h>
#include <limits.h>
#include <limits>
#include <list>
#include <map>
#include <math.h>
#include <memory>
#include <numeric>
#include <officecfg/Office/Common.hxx>
#include <osl/diagnose.h>
#include <osl/diagnose.hxx>
#include <osl/endian.h>
#include <osl/file.h>
#include <osl/file.hxx>
#include <osl/module.hxx>
#include <osl/mutex.hxx>
#include <osl/process.h>
#include <osl/signal.h>
#include <osl/thread.h>
#include <rsc/rsc-vcl-shared-types.hxx>
#include <rtl/alloc.h>
#include <rtl/bootstrap.hxx>
#include <rtl/character.hxx>
#include <rtl/crc.h>
#include <rtl/digest.h>
#include <rtl/instance.hxx>
#include <rtl/math.h>
#include <rtl/math.hxx>
#include <rtl/process.h>
#include <rtl/ref.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/tencinfo.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <rtl/uuid.h>
#include <sal/alloca.h>
#include <sal/config.h>
#include <sal/log.hxx>
#include <sal/macros.h>
#include <sal/types.h>
#include <set>
#include <setjmp.h>
#include <sot/exchange.hxx>
#include <sot/factory.hxx>
#include <sot/formats.hxx>
#include <sot/object.hxx>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <svl/brdcst.hxx>
#include <svl/ctloptions.hxx>
#include <svl/lstner.hxx>
#include <svl/smplhint.hxx>
#include <svl/undo.hxx>
#include <svl/urlbmk.hxx>
#include <tools/bigint.hxx>
#include <tools/color.hxx>
#include <tools/config.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/fontenum.hxx>
#include <tools/gen.hxx>
#include <tools/helpers.hxx>
#include <tools/line.hxx>
#include <tools/lineend.hxx>
#include <tools/poly.hxx>
#include <tools/rc.h>
#include <tools/rc.hxx>
#include <tools/rcid.h>
#include <tools/resary.hxx>
#include <tools/resid.hxx>
#include <tools/resmgr.hxx>
#include <tools/rtti.hxx>
#include <tools/stream.hxx>
#include <tools/tenccvt.hxx>
#include <tools/time.hxx>
#include <tools/urlobj.hxx>
#include <tools/vcompat.hxx>
#include <tools/zcodec.hxx>
#include <touch/touch.h>
#include <typeinfo>
#include <ucbhelper/content.hxx>
#include <unicode/ubidi.h>
#include <unicode/uchar.h>
#include <uno/current_context.hxx>
#include <uno/dispatcher.h>
#include <uno/mapping.hxx>
#include <unotools/calendarwrapper.hxx>
#include <unotools/charclass.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/confignode.hxx>
#include <unotools/fontcvt.hxx>
#include <unotools/fontdefs.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/misccfg.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/textsearch.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <utility>
#include <vector>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
