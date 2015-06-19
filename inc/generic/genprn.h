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

#ifndef INCLUDED_VCL_INC_GENERIC_GENPRN_H
#define INCLUDED_VCL_INC_GENERIC_GENPRN_H

#include "vcl/jobdata.hxx"
#include "generic/printergfx.hxx"
#include "generic/printerjob.hxx"
#include "salprn.hxx"
#include "vclpluginapi.h"

class GenPspGraphics;
class VCL_DLLPUBLIC PspSalInfoPrinter : public SalInfoPrinter
{
public:
    GenPspGraphics*         m_pGraphics;
    psp::JobData            m_aJobData;
    psp::PrinterGfx         m_aPrinterGfx;

    PspSalInfoPrinter();
    virtual ~PspSalInfoPrinter();

    // overload all pure virtual methods
    virtual SalGraphics*            AcquireGraphics() SAL_OVERRIDE;
    virtual void                    ReleaseGraphics( SalGraphics* pGraphics ) SAL_OVERRIDE;
    virtual bool                    Setup( SalFrame* pFrame, ImplJobSetup* pSetupData ) SAL_OVERRIDE;
    virtual bool                    SetPrinterData( ImplJobSetup* pSetupData ) SAL_OVERRIDE;
    virtual bool                    SetData( sal_uIntPtr nFlags, ImplJobSetup* pSetupData ) SAL_OVERRIDE;
    virtual void                    GetPageInfo( const ImplJobSetup* pSetupData,
                                                 long& rOutWidth, long& rOutHeight,
                                                 long& rPageOffX, long& rPageOffY,
                                                 long& rPageWidth, long& rPageHeight ) SAL_OVERRIDE;
    virtual sal_uIntPtr                 GetCapabilities( const ImplJobSetup* pSetupData, sal_uInt16 nType ) SAL_OVERRIDE;
    virtual sal_uIntPtr                 GetPaperBinCount( const ImplJobSetup* pSetupData ) SAL_OVERRIDE;
    virtual OUString                  GetPaperBinName( const ImplJobSetup* pSetupData, sal_uIntPtr nPaperBin ) SAL_OVERRIDE;
    virtual void                    InitPaperFormats( const ImplJobSetup* pSetupData ) SAL_OVERRIDE;
    virtual int                 GetLandscapeAngle( const ImplJobSetup* pSetupData ) SAL_OVERRIDE;
};

class VCL_DLLPUBLIC PspSalPrinter : public SalPrinter
{
public:
    OUString                  m_aFileName;
    OUString                  m_aTmpFile;
    bool                    m_bPdf:1;
    bool                    m_bIsPDFWriterJob:1;
    GenPspGraphics*         m_pGraphics;
    psp::PrinterJob         m_aPrintJob;
    psp::JobData            m_aJobData;
    psp::PrinterGfx         m_aPrinterGfx;
    sal_uIntPtr                 m_nCopies;
    bool                    m_bCollate;
    SalInfoPrinter*         m_pInfoPrinter;

    PspSalPrinter( SalInfoPrinter *pPrinter );
    virtual ~PspSalPrinter();

    // overload all pure virtual methods
    virtual bool                    StartJob( const OUString* pFileName,
                                              const OUString& rJobName,
                                              const OUString& rAppName,
                                              sal_uIntPtr nCopies,
                                              bool bCollate,
                                              bool bDirect,
                                              ImplJobSetup* pSetupData ) SAL_OVERRIDE;
    virtual bool                    StartJob( const OUString*,
                                              const OUString&,
                                              const OUString&,
                                              ImplJobSetup*,
                                              vcl::PrinterController& i_rController ) SAL_OVERRIDE;
    virtual bool                    EndJob() SAL_OVERRIDE;
    virtual bool                    AbortJob() SAL_OVERRIDE;
    virtual SalGraphics*            StartPage( ImplJobSetup* pSetupData, bool bNewJobData ) SAL_OVERRIDE;
    virtual bool                    EndPage() SAL_OVERRIDE;
    virtual sal_uIntPtr                 GetErrorCode() SAL_OVERRIDE;
};

#endif // INCLUDED_VCL_INC_GENERIC_GENPRN_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
