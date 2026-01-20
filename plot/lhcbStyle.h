#ifndef LHCBSTYLE_H
#define LHCBSTYLE_H

#include "TStyle.h"
#include "TROOT.h"
#include "TPaveText.h"
#include "TText.h"
#include "TLatex.h"
#include <iostream>

inline void lhcbStyle() {
    Int_t lhcbFont     = 132;
    Double_t lhcbWidth = 1.00;
    Double_t lhcbTSize = 0.045;

    TStyle* lhcbStyle = new TStyle("lhcbStyle","LHCb plots style");
    lhcbStyle->SetFillColor(1);
    lhcbStyle->SetFillStyle(1001);
    lhcbStyle->SetFrameFillColor(0);
    lhcbStyle->SetFrameBorderMode(0);
    lhcbStyle->SetPadBorderMode(0);
    lhcbStyle->SetPadColor(0);
    lhcbStyle->SetCanvasBorderMode(0);
    lhcbStyle->SetCanvasColor(0);
    lhcbStyle->SetStatColor(0);
    lhcbStyle->SetLegendBorderSize(0);
    lhcbStyle->SetPalette(1);
    lhcbStyle->SetPaperSize(20,26);
    lhcbStyle->SetPadTopMargin(0.12);
    lhcbStyle->SetPadRightMargin(0.17);
    lhcbStyle->SetPadBottomMargin(0.23);
    lhcbStyle->SetPadLeftMargin(0.17);
    lhcbStyle->SetTextFont(lhcbFont);
    lhcbStyle->SetTextSize(lhcbTSize);
    lhcbStyle->SetLabelFont(lhcbFont,"x");
    lhcbStyle->SetLabelFont(lhcbFont,"y");
    lhcbStyle->SetLabelFont(lhcbFont,"z");
    lhcbStyle->SetLabelSize(lhcbTSize,"x");
    lhcbStyle->SetLabelSize(lhcbTSize,"y");
    lhcbStyle->SetLabelSize(lhcbTSize,"z");
    lhcbStyle->SetTitleFont(lhcbFont);
    lhcbStyle->SetTitleFont(lhcbFont,"x");
    lhcbStyle->SetTitleFont(lhcbFont,"y");
    lhcbStyle->SetTitleFont(lhcbFont,"z");
    lhcbStyle->SetTitleSize(1.2*lhcbTSize,"x");
    lhcbStyle->SetTitleSize(1.2*lhcbTSize,"y");
    lhcbStyle->SetTitleSize(1.2*lhcbTSize,"z");
    lhcbStyle->SetLineWidth(lhcbWidth);
    lhcbStyle->SetFrameLineWidth(lhcbWidth);
    lhcbStyle->SetHistLineWidth(lhcbWidth);
    lhcbStyle->SetFuncWidth(lhcbWidth);
    lhcbStyle->SetGridWidth(lhcbWidth);
    lhcbStyle->SetLineStyleString(2,"[12 12]");
    lhcbStyle->SetMarkerStyle(20);
    lhcbStyle->SetMarkerSize(0.2);
    lhcbStyle->SetLabelOffset(0.010,"X");
    lhcbStyle->SetLabelOffset(0.010,"Y");
    lhcbStyle->SetOptStat(0);
    lhcbStyle->SetStatFormat("6.3g");
    lhcbStyle->SetOptTitle(0);
    lhcbStyle->SetOptFit(0);
    lhcbStyle->SetTitleOffset(1.0,"X");
    lhcbStyle->SetTitleOffset(1.3,"Y");
    lhcbStyle->SetTitleOffset(1.2,"Z");
    lhcbStyle->SetTitleFillColor(0);
    lhcbStyle->SetTitleStyle(0);
    lhcbStyle->SetTitleBorderSize(0);
    lhcbStyle->SetTitleFont(lhcbFont,"title");
    lhcbStyle->SetTitleX(0.0);
    lhcbStyle->SetTitleY(1.0); 
    lhcbStyle->SetTitleW(1.0);
    lhcbStyle->SetTitleH(0.05);
    lhcbStyle->SetLegendTextSize(0.055);
    lhcbStyle->SetLegendFont(132);
    lhcbStyle->SetStatBorderSize(0);
    lhcbStyle->SetStatFont(lhcbFont);
    lhcbStyle->SetStatFontSize(0.05);
    lhcbStyle->SetStatX(0.9);
    lhcbStyle->SetStatY(0.9);
    lhcbStyle->SetStatW(0.25);
    lhcbStyle->SetStatH(0.15);
    lhcbStyle->SetPadTickX(1);
    lhcbStyle->SetPadTickY(0);
    lhcbStyle->SetNdivisions(505,"x");
    lhcbStyle->SetNdivisions(510,"y");

    gROOT->SetStyle("lhcbStyle");
    gROOT->ForceStyle();

    gStyle->SetPalette(kAvocado);

    std::cout << "-------------------------" << std::endl;  
    std::cout << "Set LHCb Style - via C++" << std::endl;
    std::cout << "-------------------------" << std::endl;
}

#endif
