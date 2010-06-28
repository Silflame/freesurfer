/**
 * @file  ToolWindowMeasure.h
 * @brief Preferences dialog.
 *
 */
/*
 * Original Author: Ruopeng Wang
 * CVS Revision Info:
 *    $Author: rpwang $
 *    $Date: 2010/06/28 20:09:41 $
 *    $Revision: 1.9 $
 *
 * Copyright (C) 2008-2009,
 * The General Hospital Corporation (Boston, MA).
 * All rights reserved.
 *
 * Distribution, usage and copying of this software is covered under the
 * terms found in the License Agreement file named 'COPYING' found in the
 * FreeSurfer source code root directory, and duplicated here:
 * https://surfer.nmr.mgh.harvard.edu/fswiki/FreeSurferOpenSourceLicense
 *
 * General inquiries: freesurfer@nmr.mgh.harvard.edu
 * Bug reports: analysis-bugs@nmr.mgh.harvard.edu
 *
 */



#include "ToolWindowMeasure.h"
#include <wx/wx.h>
#include <wx/config.h>
#include <wx/xrc/xmlres.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/spinctrl.h>
#include <wx/listbox.h>
#include <wx/clipbrd.h>
#include <wx/ffile.h>
#include "MainWindow.h"
#include "RenderView2D.h"
#include "RenderView3D.h"
#include "BrushProperty.h"
#include "Interactor2DMeasure.h"
#include "LayerCollection.h"
#include "LayerMRI.h"
#include "LayerPropertiesMRI.h"
#include "Region2D.h"
#include "SurfaceRegion.h"

BEGIN_EVENT_TABLE( ToolWindowMeasure, wxFrame )
  EVT_MENU      ( XRCID( "ID_ACTION_MEASURE_LINE" ),      ToolWindowMeasure::OnActionMeasureLine )
  EVT_UPDATE_UI ( XRCID( "ID_ACTION_MEASURE_LINE" ),      ToolWindowMeasure::OnActionMeasureLineUpdateUI )
  EVT_MENU      ( XRCID( "ID_ACTION_MEASURE_RECT" ),      ToolWindowMeasure::OnActionMeasureRectangle )
  EVT_UPDATE_UI ( XRCID( "ID_ACTION_MEASURE_RECT" ),      ToolWindowMeasure::OnActionMeasureRectangleUpdateUI )
  EVT_MENU      ( XRCID( "ID_ACTION_MEASURE_POLYLINE" ),  ToolWindowMeasure::OnActionMeasurePolyline )
  EVT_UPDATE_UI ( XRCID( "ID_ACTION_MEASURE_POLYLINE" ),  ToolWindowMeasure::OnActionMeasurePolylineUpdateUI )
  EVT_MENU      ( XRCID( "ID_ACTION_MEASURE_SPLINE" ),    ToolWindowMeasure::OnActionMeasureSpline )
  EVT_UPDATE_UI ( XRCID( "ID_ACTION_MEASURE_SPLINE" ),    ToolWindowMeasure::OnActionMeasureSplineUpdateUI )
  EVT_MENU      ( XRCID( "ID_ACTION_MEASURE_SURFACE" ),   ToolWindowMeasure::OnActionMeasureSurfaceRegion )
  EVT_UPDATE_UI ( XRCID( "ID_ACTION_MEASURE_SURFACE" ),   ToolWindowMeasure::OnActionMeasureSurfaceRegionUpdateUI )
  EVT_BUTTON    ( XRCID( "ID_BUTTON_COPY" ),              ToolWindowMeasure::OnButtonCopy )
  EVT_BUTTON    ( XRCID( "ID_BUTTON_EXPORT" ),            ToolWindowMeasure::OnButtonExport )
  EVT_BUTTON    ( XRCID( "ID_BUTTON_SAVE" ),              ToolWindowMeasure::OnButtonSave )
  EVT_BUTTON    ( XRCID( "ID_BUTTON_SAVE_ALL" ),          ToolWindowMeasure::OnButtonSaveAll )
  EVT_BUTTON    ( XRCID( "ID_BUTTON_LOAD" ),              ToolWindowMeasure::OnButtonLoad )
  EVT_SPINCTRL  ( XRCID( "ID_SPIN_ID"),                   ToolWindowMeasure::OnSpinId )      
  
  EVT_SHOW      ( ToolWindowMeasure::OnShow )

END_EVENT_TABLE()


ToolWindowMeasure::ToolWindowMeasure( wxWindow* parent ) : Listener( "ToolWindowMeasure" )
{
  wxXmlResource::Get()->LoadFrame( this, parent, wxT("ID_TOOLWINDOW_MEASURE") );
  m_toolbar     = XRCCTRL( *this, "ID_TOOLBAR_MEASURE", wxToolBar );
  m_textStats   = XRCCTRL( *this, "ID_TEXT_STATS",      wxTextCtrl );
  m_btnCopy     = XRCCTRL( *this, "ID_BUTTON_COPY",     wxButton );
  m_btnExport   = XRCCTRL( *this, "ID_BUTTON_EXPORT",   wxButton );
  m_btnSave     = XRCCTRL( *this, "ID_BUTTON_SAVE",     wxButton );
  m_btnSaveAll  = XRCCTRL( *this, "ID_BUTTON_SAVE_ALL",   wxButton );
  m_btnLoad     = XRCCTRL( *this, "ID_BUTTON_LOAD",     wxButton );
  m_spinId      = XRCCTRL( *this, "ID_SPIN_ID",         wxSpinCtrl );
  
  m_widgets2D.push_back( m_btnCopy );
  m_widgets2D.push_back( m_btnExport );
  
  m_widgets3D.push_back( m_btnSave );
  m_widgets3D.push_back( m_btnSaveAll );
  m_widgets3D.push_back( m_btnLoad );
  m_widgets3D.push_back( m_spinId );
  m_widgets3D.push_back( XRCCTRL( *this, "ID_STATIC_ID",    wxStaticText ) );
  
  m_region = NULL;
  m_surfaceRegion = NULL;
  m_bToUpdateWidgets = true;
}

ToolWindowMeasure::~ToolWindowMeasure()
{}

void ToolWindowMeasure::OnShow( wxShowEvent& event )
{
  if ( event.GetShow() )
  {
    wxConfigBase* config = wxConfigBase::Get();
    if ( config )
    {
      int x = config->Read( _T("/ToolWindowMeasure/PosX"), 0L );
      int y = config->Read( _T("/ToolWindowMeasure/PosY"), 0L );
      if ( x == 0 && y == 0 )
        Center();
      else
        Move( x, y );
    }
    
 //   SetClientSize( GetClientSize().GetWidth(), m_toolbar->GetSize().GetHeight() );
  }
  else
  {
    wxConfigBase* config = wxConfigBase::Get();
    if ( config )
    {
      int x, y;
      GetPosition( &x, &y );
      config->Write( _T("/ToolWindowMeasure/PosX"), (long) x );
      config->Write( _T("/ToolWindowMeasure/PosY"), (long) y );
    }
  }
}

void ToolWindowMeasure::SetRegion( Region2D* reg )
{
  m_region = reg;
  if ( m_region )
  {
    m_region->AddListener( this );
    m_surfaceRegion = NULL;
    RenderView* view = MainWindow::GetMainWindowPointer()->GetRenderView( 0 );
    if ( view->GetAction() == Interactor::MM_SurfaceRegion )
      MainWindow::GetMainWindowPointer()->SetAction( Interactor::MM_Line );
  }  
  UpdateWidgets();
}

void ToolWindowMeasure::SetSurfaceRegion( SurfaceRegion* reg )
{
  m_surfaceRegion = reg;
  if ( m_surfaceRegion )
  {
    m_surfaceRegion->AddListener( this );
    m_region = NULL;
    MainWindow::GetMainWindowPointer()->SetAction( Interactor::MM_SurfaceRegion );
  }
  UpdateWidgets();
}

void ToolWindowMeasure::UpdateWidgets( )
{
  m_bToUpdateWidgets = true;
}

void ToolWindowMeasure::DoUpdateWidgets()
{
  wxString strg;
  if ( m_region )
  {
    wxArrayString strgs = m_region->GetLongStats();
    for ( size_t i = 0; i < strgs.size(); i++ )
      strg += strgs[i] + "\n";   
  }
  m_textStats->ChangeValue( strg ); 
  m_btnCopy->Enable( !strg.IsEmpty() );
  m_btnExport->Enable( !strg.IsEmpty() );
  
  RenderView3D* view = (RenderView3D*)MainWindow::GetMainWindowPointer()->GetRenderView( 3 );
  for ( size_t i = 0; i < m_widgets3D.size(); i++ )
    m_widgets3D[i]->Show( view->GetAction() == Interactor::MM_SurfaceRegion );
  
  for ( size_t i = 0; i < m_widgets2D.size(); i++ )
    m_widgets2D[i]->Show( view->GetAction() != Interactor::MM_SurfaceRegion );
  
  if ( m_surfaceRegion )
    m_spinId->SetValue( m_surfaceRegion->GetId() );
  
  LayerMRI* mri = (LayerMRI*)MainWindow::GetMainWindowPointer()->GetActiveLayer( "MRI" );
  bool bSurfaceRegionValid = ( mri && mri->GetProperties()->GetShowAsContour() && mri->GetNumberOfSurfaceRegions() > 0 );
  if ( bSurfaceRegionValid )
    m_spinId->SetRange( 1, mri->GetNumberOfSurfaceRegions() );
  
  m_btnSave->Enable( m_surfaceRegion && bSurfaceRegionValid );
  m_spinId->Enable( m_surfaceRegion && bSurfaceRegionValid );
  m_btnSaveAll->Enable( bSurfaceRegionValid );
  
  XRCCTRL( *this, "ID_PANEL_HOLDER",  wxPanel )->Layout();
  Layout();
  m_bToUpdateWidgets = false;
}

void ToolWindowMeasure::OnInternalIdle()
{
  wxFrame::OnInternalIdle();
  
  if ( m_bToUpdateWidgets )
    DoUpdateWidgets();
}

void ToolWindowMeasure::DoListenToMessage ( std::string const iMsg, void* iData, void* sender )
{
  if ( iMsg == "RegionStatsUpdated" )
  {
    if ( m_region == iData || m_region == sender )
      UpdateWidgets();
  }
  else if ( iMsg == "RegionSelected" )
  {
    SetRegion( (Region2D*)iData );
  }
  else if ( iMsg == "RegionRemoved" )
  {
    if ( m_region == iData )
      SetRegion( NULL );
  }
  else if ( iMsg == "SurfaceRegionSelected" )
  {
    SetSurfaceRegion( (SurfaceRegion*)iData );
  }
  else if ( iMsg == "SurfaceRegionRemoved" )
  {
    if ( m_surfaceRegion == iData )
      SetSurfaceRegion( NULL );
  }
}

void ToolWindowMeasure::OnActionMeasureLine( wxCommandEvent& event )
{
  MainWindow::GetMainWindowPointer()->SetAction( Interactor::MM_Line );
  UpdateWidgets();
}

void ToolWindowMeasure::OnActionMeasureLineUpdateUI( wxUpdateUIEvent& event)
{
  RenderView2D* view = ( RenderView2D* )MainWindow::GetMainWindowPointer()->GetRenderView( 0 );
  event.Check( view->GetInteractionMode() == RenderView2D::IM_Measure
      && view->GetAction() == Interactor::MM_Line );
  event.Enable( view->GetInteractionMode() == RenderView2D::IM_Measure
                && !MainWindow::GetMainWindowPointer()->GetLayerCollection( "MRI" )->IsEmpty() );
}


void ToolWindowMeasure::OnActionMeasureRectangle( wxCommandEvent& event )
{
  MainWindow::GetMainWindowPointer()->SetAction( Interactor::MM_Rectangle );
  UpdateWidgets();
}

void ToolWindowMeasure::OnActionMeasureRectangleUpdateUI( wxUpdateUIEvent& event)
{
  RenderView2D* view = ( RenderView2D* )MainWindow::GetMainWindowPointer()->GetRenderView( 0 );
  event.Check( view->GetInteractionMode() == RenderView2D::IM_Measure
      && view->GetAction() == Interactor::MM_Rectangle );
  event.Enable( view->GetInteractionMode() == RenderView2D::IM_Measure
      && !MainWindow::GetMainWindowPointer()->GetLayerCollection( "MRI" )->IsEmpty() );
}

void ToolWindowMeasure::OnActionMeasurePolyline( wxCommandEvent& event )
{
  MainWindow::GetMainWindowPointer()->SetAction( Interactor::MM_Polyline );
  UpdateWidgets();
}

void ToolWindowMeasure::OnActionMeasurePolylineUpdateUI( wxUpdateUIEvent& event)
{
  RenderView2D* view = ( RenderView2D* )MainWindow::GetMainWindowPointer()->GetRenderView( 0 );
  event.Check( view->GetInteractionMode() == RenderView2D::IM_Measure
      && view->GetAction() == Interactor::MM_Polyline );
  event.Enable( view->GetInteractionMode() == RenderView2D::IM_Measure
      && !MainWindow::GetMainWindowPointer()->GetLayerCollection( "MRI" )->IsEmpty() );
}

void ToolWindowMeasure::OnActionMeasureSpline( wxCommandEvent& event )
{
  MainWindow::GetMainWindowPointer()->SetAction( Interactor::MM_Spline );
  UpdateWidgets();
}

void ToolWindowMeasure::OnActionMeasureSplineUpdateUI( wxUpdateUIEvent& event)
{
  RenderView2D* view = ( RenderView2D* )MainWindow::GetMainWindowPointer()->GetRenderView( 0 );
  event.Check( view->GetInteractionMode() == RenderView2D::IM_Measure
      && view->GetAction() == Interactor::MM_Spline );
  event.Enable( view->GetInteractionMode() == RenderView2D::IM_Measure
      && !MainWindow::GetMainWindowPointer()->GetLayerCollection( "MRI" )->IsEmpty() );
}

void ToolWindowMeasure::OnActionMeasureSurfaceRegion( wxCommandEvent& event )
{
  MainWindow::GetMainWindowPointer()->SetAction( Interactor::MM_SurfaceRegion );
  UpdateWidgets();
}

void ToolWindowMeasure::OnActionMeasureSurfaceRegionUpdateUI( wxUpdateUIEvent& event)
{
  RenderView2D* view = ( RenderView2D* )MainWindow::GetMainWindowPointer()->GetRenderView( 0 );
  event.Check( view->GetInteractionMode() == RenderView2D::IM_Measure
      && view->GetAction() == Interactor::MM_SurfaceRegion );
  event.Enable( view->GetInteractionMode() == RenderView2D::IM_Measure
      && !MainWindow::GetMainWindowPointer()->GetLayerCollection( "MRI" )->IsEmpty() );
}

void ToolWindowMeasure::OnButtonCopy( wxCommandEvent& event )
{
  wxString output = m_textStats->GetValue();
  if (wxTheClipboard->Open())
  {
    wxTheClipboard->SetData( new wxTextDataObject( output ) );
    wxTheClipboard->Close();
  }
}

void ToolWindowMeasure::OnButtonExport( wxCommandEvent& event )
{
  wxFileDialog dlg( this, _("Export to file"), _(""), _(""),
                    _("All files (*.*)|*.*"),
                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
  if ( dlg.ShowModal() == wxID_OK )
  {
    wxString fn = dlg.GetPath();
    if ( !m_textStats->SaveFile( fn ) )
    {
      wxMessageDialog msg_dlg( this, wxString("Can not write to file ") + fn, 
                           _("Error"), wxOK );
      msg_dlg.ShowModal();
    } 
  }
}

void ToolWindowMeasure::OnSpinId( wxSpinEvent& event )
{
  RenderView3D* view = ( RenderView3D* )MainWindow::GetMainWindowPointer()->GetRenderView( 3 );
  view->PickSelectRegion( event.GetInt() );
}

void ToolWindowMeasure::OnButtonSave( wxCommandEvent& event )
{
  wxFileDialog dlg( this, _("Save region to file"), _(""), _(""),
                    _("All files (*.*)|*.*"),
                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
  if ( m_surfaceRegion && dlg.ShowModal() == wxID_OK )
  {
    wxString fn = dlg.GetPath();
    if ( !m_surfaceRegion->Write( fn ) )
    {
      wxMessageDialog msg_dlg( this, wxString("Can not write to file ") + fn, 
                               _("Error"), wxOK );
      msg_dlg.ShowModal();
    } 
  }
}

void ToolWindowMeasure::OnButtonSaveAll( wxCommandEvent& event )
{
  wxFileDialog dlg( this, _("Save all regions to file"), _(""), _(""),
                    _("All files (*.*)|*.*"),
                    wxFD_SAVE | wxFD_OVERWRITE_PROMPT );
  if ( dlg.ShowModal() == wxID_OK )
  {
    wxString fn = dlg.GetPath();
    LayerMRI* mri = (LayerMRI*)MainWindow::GetMainWindowPointer()->GetActiveLayer( "MRI" );
    if ( mri && !mri->SaveAllSurfaceRegions( fn ) )
    {
      wxMessageDialog msg_dlg( this, wxString("Can not write to file ") + fn, 
                               _("Error"), wxOK );
      msg_dlg.ShowModal();
    } 
  }
}

void ToolWindowMeasure::OnButtonLoad( wxCommandEvent& event )
{
  wxFileDialog dlg( this, _("Load region(s) from file"), _(""), _(""),
                    _("All files (*.*)|*.*"),
                    wxFD_OPEN );
  
  LayerMRI* mri = (LayerMRI*)MainWindow::GetMainWindowPointer()->GetActiveLayer( "MRI" );
  if ( mri && dlg.ShowModal() == wxID_OK )
  {
    wxString fn = dlg.GetPath();
    if ( !mri->LoadRegionSurfaces( fn ) )
    {
      wxMessageDialog msg_dlg( this, wxString("Can not load file ") + fn, 
                               _("Error"), wxOK );
      msg_dlg.ShowModal();
    } 
    UpdateWidgets();
  }
}
