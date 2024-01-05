#ifndef _WXINCLUDES_H_
#	define _WXINCLUDES_H_

#	pragma once

#	include <wx/wxprec.h>

#	ifdef __BORLANDC__
#		pragma hdrstop
#	endif

#	ifndef WX_PRECOMP
#		include <wx/wx.h>
#	endif

#	include "wx/wx.h"
#	include "wx/log.h"
#	include "wx/config.h"
#	include "wx/app.h"
#	include "wx/frame.h"
#	include "wx/config.h"
#	include "wx/fileconf.h"
#	include "wx/image.h"
#	include "wx/bitmap.h"
#	include "wx/gbsizer.h"
#	include "wx/thread.h"
#	include "wx/progdlg.h"
#	include "wx/splash.h"
#	include "wx/dcbuffer.h"
#	include "wx/dynlib.h"
#	include "wx/sysopt.h"
#	include "wx/mstream.h"
#	include "wx/filename.h"
#	include "wx/fs_mem.h"
#	include "wx/fs_zip.h"
#	include "wx/textdlg.h"
#	include "wx/zipstrm.h"
#	include "wx/snglinst.h"
#	include "wx/tooltip.h"
#	include "wx/wizard.h"
#	include "wx/busyinfo.h"
#	include "wx/popupwin.h"
#	include "wx/uri.h"
#	include "wx/printdlg.h"
#	include "wx/print.h"
#	include "wx/tokenzr.h"

// Debug reporting
#	include "wx/debugrpt.h"

// Drag and drop
#	include "wx/dnd.h"
#	include "wx/dataobj.h"

#ifndef _STRIP

// wxDockIT
//#	include "wx/layoutmanager.h"
//#	include "wx/dockwindow.h"
//#	include "wx/dockpanel.h"
//#	include "wx/dockhost.h"
//#	include "wx/pane.h"
//#	include "wx/util.h"
//#	include "wx/slidebar.h"
//#	include "wx/toolbutton.h"

// wxPropertyGrid
//#	include "wx/propgrid/manager.h"
//#	include "wx/propgrid/propgrid.h"
//#	include "wx/propgrid/propdev.h"

#endif

// XRC
#	include "wx/xrc/xmlres.h"

// Local utils
//#	include "util.h"

// Shell32
#ifdef _WIN32
#	include "shlobj.h"
#endif

//global macro
#define MIN(a,b) (((a)<=(b)) ? (a) : (b))
#define MAX(a,b) (((a)>=(b)) ? (a) : (b))

#endif
