/*
Launchy: Application Launcher
Copyright (C) 2005  Josh Karlin

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// SmartComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "Launchy.h"
#include "SmartComboBox.h"
#include "LaunchyDlg.h"
#include ".\smartcombobox.h"
#include <algorithm>

// SmartComboBox

IMPLEMENT_DYNAMIC(SmartComboBox, CComboBox)

SmartComboBox::SmartComboBox()
: typed(_T("")), searchPath(_T(""))
{
	m_RemoveFrame = false;
	m_RemoveButton = false;
	cloneSelect = -1;

	CComboBox();
}

SmartComboBox::~SmartComboBox()
{
}


BEGIN_MESSAGE_MAP(SmartComboBox, CComboBox)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_DRAWITEM_REFLECT()

	ON_CONTROL_REFLECT(CBN_EDITUPDATE, &SmartComboBox::OnCbnEditupdate)
//	ON_CONTROL_REFLECT(CBN_SELCHANGE, &SmartComboBox::OnSelEndOK)
	ON_CONTROL_REFLECT(CBN_CLOSEUP, &SmartComboBox::OnCbnCloseup)
	ON_CONTROL_REFLECT(CBN_EDITCHANGE, &SmartComboBox::OnCbnEditchange)
	ON_CONTROL_REFLECT(CBN_SELENDOK, &SmartComboBox::OnSelEndOK)
	ON_CONTROL_REFLECT(CBN_DROPDOWN, &SmartComboBox::OnCbnDropdown)
//	ON_MESSAGE(WM_CHANGE_COMBO_SEL, &SmartComboBox::AfterSelChange)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
END_MESSAGE_MAP()



// SmartComboBox message handlers




HBRUSH SmartComboBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{


	//HBRUSH hbr = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);

	pDC->SetTextColor(m_crText);
	pDC->SetBkColor(m_crBackGnd);
	pDC->SetBkMode(TRANSPARENT);


	if (nCtlColor == CTLCOLOR_EDIT)
	{

		if (m_Transparent) {
			CBrush m_Brush;
			m_Brush.CreateStockObject(HOLLOW_BRUSH);
			return (HBRUSH) m_Brush;
		}

	}



	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired


	return m_brBackGnd;
}

void SmartComboBox::SetBackColor(COLORREF rgb)
{
	//set background color ref (used for text's background)
	m_crBackGnd = rgb;

	//free brush
	if (m_brBackGnd.GetSafeHandle())
		m_brBackGnd.DeleteObject();
	//set brush to new color
	m_brBackGnd.CreateSolidBrush(rgb);

	//redraw
	//	Invalidate(TRUE);
}


void SmartComboBox::SetTextColor(COLORREF rgb)
{
	//set text color ref
	m_crText = rgb;

	//redraw
	//	Invalidate(TRUE);
}

void SmartComboBox::OnDestroy()
{


	CComboBox::OnDestroy();

	// TODO: Add your message handler code here
}

/*
	The tab key was pressed and so we should
	reformat the string
*/
void SmartComboBox::TabSearchTxt()
{
	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
	if (pDlg == NULL) return;

	if (pDlg->smarts->matches.size() > 0) {

		TabbedMatch = pDlg->smarts->matches[0];

		SearchStrings.Add(pDlg->smarts->matches[0]->croppedName);
		SearchPluginID = pDlg->smarts->matches[0]->owner;
	} else {
//		return;
		if (SearchStrings.GetCount() == 0) return;
		SearchStrings.Add(searchTxt);
	}
	searchTxt = L"";

	ShowDropDown(false);
	pDlg->smarts->Update(searchTxt);
	ReformatDisplay();
	ParseSearchTxt();
}

void SmartComboBox::DeleteLine()
{
	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
	if (pDlg == NULL) return;

	searchTxt = L"";
	SearchStrings.RemoveAll();
	ShowDropDown(false);
	pDlg->smarts->Update(searchTxt);
	ReformatDisplay();
	ParseSearchTxt();
}

void SmartComboBox::DeleteWord()
{
	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
	if (pDlg == NULL) return;

	if (searchTxt == L"" && SearchStrings.GetCount() > 0) {
		SearchStrings.RemoveAt(SearchStrings.GetCount()-1);
	} else {
		CString schars = L" \\./";
		searchTxt.TrimRight(schars);
		int max = -1;
		for(int i = 0; i < schars.GetLength(); i++) {
			TCHAR c = schars[i];
			if (searchTxt.ReverseFind(c) > max) 
				max = searchTxt.ReverseFind(c);
		}
		if (max != -1) {
			searchTxt = searchTxt.Left(max + 1);
		} else {
			searchTxt = L"";
		}
	}

	ShowDropDown(false);
	pDlg->smarts->Update(searchTxt);
	ReformatDisplay();
	ParseSearchTxt();
}


void SmartComboBox::ReformatDisplay()
{
	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
	if (pDlg == NULL) return;

	CString out = L"";

	for(int i = 0; i < SearchStrings.GetCount(); i++) {
		out += SearchStrings[i];
		if (SearchPluginID != -1)
			out += pDlg->plugins->GetSeparator(SearchPluginID);
		else
			out += L" | ";
	}
	out += searchTxt;
	this->SetWindowTextW(out);
	searchTxt = out;
	SetEditSel(out.GetLength(), out.GetLength());
	CleanText();
}

void SmartComboBox::ParseSearchTxt()
{
	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
	if (pDlg == NULL) return;

	CString sep;
	if (SearchStrings.GetCount() > 0 && SearchPluginID != -1)
		sep = pDlg->plugins->GetSeparator(SearchPluginID);
	else
		sep = L" | ";
	CStringArray NewSearchStrings;
	CString prefix = L"";

	int pos = 0;
	for(int strNum = 0; strNum < SearchStrings.GetCount(); strNum++) {
		CString prefix = SearchStrings[strNum];

		for(int charNum = 0; charNum < prefix.GetLength(); charNum++) {
			if (prefix[charNum] != searchTxt[pos]) break;
			pos += 1;
			if (charNum == prefix.GetLength() - 1)
				NewSearchStrings.Add(prefix);
		}
		for(int j = 0; j < sep.GetLength(); j++) {
			if (searchTxt[pos] == sep[j])
				pos += 1;
			else
				break;
		}
	}
	

	searchTxt = searchTxt.Right(searchTxt.GetLength() - pos);
/*
	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
	if (pDlg == NULL) return;
	if (SearchStrings.GetCount() == 0 && pDlg->smarts->matches.size() > 0 && pDlg->smarts->matches[0]->owner != -1) {
		CString tmp = searchTxtBak;
		tmp += searchTxt;
		searchTxt = tmp;
	}
	*/
	searchTxt.MakeLower();
	SearchStrings.Copy(NewSearchStrings);
}


void SmartComboBox::OnCbnEditupdate()
{

	this->GetWindowTextW(searchTxt);	
	ParseSearchTxt();

	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
	if (pDlg == NULL) return;

	pDlg->smarts->Update(searchTxt);
}




void SmartComboBox::OnCbnEditchange()
{
	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
	if (pDlg == NULL) return;



	this->GetWindowTextW(typed);
	searchPath = pDlg->smarts->GetMatchPath(0);
}

void SmartComboBox::OnSelEndOK()
{
	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
	if (pDlg == NULL) return;


	
	// Send the CB_GETCURSEL message so that we can get the index
	GetWindowTextW(searchTxt);
	int index = this->SendMessage(CB_GETCURSEL);
	if (index == CB_ERR) 
		searchTxt = L"";
	else
		this->GetLBText(index, searchTxt);	

	ReformatDisplay();
	ParseSearchTxt();
	//CleanText();

	pDlg->smarts->Update(searchTxt,false);

//	this->PostMessage(WM_CHANGE_COMBO_SEL,IDC_Input,(LPARAM)(bool) false);
	// If it's closing, we've already taken care of this..
/*	if (GetDroppedState()) {
		int sel = m_listbox.GetCurSel();

		if (sel != LB_ERR) {
			m_listbox.GetText(m_listbox.GetCurSel(), searchTxt);
			ParseSearchTxt();
			DropItem* data = (DropItem*) GetItemDataPtr(sel);
			pDlg->smarts->Update(searchTxt,false, data->longpath);
		}
	}
	*/
}


void SmartComboBox::OnCbnDropdown()
{
	// The following line tells the mouse cursor to pop up
	// http://support.microsoft.com/kb/326254
	SendMessage(WM_SETCURSOR,0,0);

	//	SmartComboBox* pmyComboBox = this;
	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
//	SetCurSel(-1);

	// Find the longest string in the combo box.
	CString str;
	CSize   sx,sz;
	int     dx=0;
	CDC*    pDC = GetDC();
	pDC->SelectObject(m_FontSmall);
	CString cStr;
	for (int i=0;i < GetCount();i++)
	{
		this->SetItemHeight(i, 40);
		//		pmyComboBox->GetLBText( i, str );		
		//		sz = pDC->GetTextExtent(str);
		DropItem* d = (DropItem*) GetItemDataPtr(i);
		sz = pDC->GetTextExtent(d->lesspath);

		this->GetLBText(i, cStr);
		sx = pDC->GetOutputTextExtent(cStr);

		if (sz.cx > sx.cx) {
			if (sz.cx > dx) {
				dx = sz.cx;
			}
		} else {
			if (sx.cx > dx) {
				dx = sx.cx;
			}
		}
	}
	ReleaseDC(pDC);


	// Adjust the width for the vertical scroll bar and the left and
	// right border.
	dx += ::GetSystemMetrics(SM_CXVSCROLL) + 62;// + 2*::GetSystemMetrics(SM_CXEDGE);

	// Set the width of the list box so that every item is completely visible.
	SetDroppedWidth(dx);


	for(int i = 0; i < GetCount(); i++) {
		DropItem* data = (DropItem*) GetItemDataPtr(i);
		if (data->longpath.Find(_T(".directory")) != -1) {
			data->icon = pDlg->IconInfo.GetFolderIconHandle(false);
		} else {
			if (data->owner != -1) {
				data->icon = pDlg->plugins->GetIcon(data->owner);
			} else {
				data->icon = pDlg->IconInfo.GetIconHandleNoOverlay(data->longpath, false);
			}
		}			
	}
}





void SmartComboBox::CleanText(void)
{
	if (m_Transparent) {
		CWnd* pParent = GetParent();
		if (pParent == NULL) return;
		CRect rect;
		GetWindowRect(rect);
		pParent->ScreenToClient(rect);
		rect.DeflateRect(2, 2);

		pParent->InvalidateRect(rect, TRUE); 
	}
}



void SmartComboBox::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO: Add your message handler code here and/or call default
	lpMeasureItemStruct->itemHeight = 40;
}


void SmartComboBox::SetSmallFont(CFont* font, COLORREF rgb)
{
	m_FontSmall = font;	
	m_FontSmallRGB = rgb;
}




LRESULT SmartComboBox::AfterSelChange(UINT wParam, LONG lParam) {
	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
	if (pDlg == NULL) return true;

	// Send the CB_GETCURSEL message so that we can get the index
	int index = this->SendMessage(CB_GETCURSEL);
	if (index == CB_ERR) 
		searchTxt = L"";
	else
		this->GetLBText(index, searchTxt);
//	this->GetWindowTextW(searchTxt);

	ReformatDisplay();
	ParseSearchTxt();
	//CleanText();

	pDlg->smarts->Update(searchTxt, lParam != 0);

	
	return true;
}




void SmartComboBox::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
//	OnDrawSelchange((int)lpDrawItemStruct->itemID);
	CRect rItem;
	CRect rText;
	CRect rIcon;
	CDC* dc = CDC::FromHandle(lpDrawItemStruct->hDC);

	if ((int)lpDrawItemStruct->itemID < 0)
	{
		// If there are no elements in the List Box
		// based on whether the list box has Focus or not
		// draw the Focus Rect or Erase it,
		if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && (lpDrawItemStruct->itemState & ODS_FOCUS))
		{
			dc->DrawFocusRect(&lpDrawItemStruct->rcItem);
		}
		else if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && !(lpDrawItemStruct->itemState & ODS_FOCUS))
		{
			dc->DrawFocusRect(&lpDrawItemStruct->rcItem);
		}
		return;
	}

	// String to store the text, which will be added to the CListBox
	CString strText;

	// Get the item text.
	
	this->GetLBText((int)lpDrawItemStruct->itemID, strText);
	//	AfxMessageBox(strText);

	//Initialize the Item's row
	rItem = lpDrawItemStruct->rcItem;

	//The icon that the sample has created has a width of 32 pixels
	rIcon = lpDrawItemStruct->rcItem;
	rIcon.bottom = rIcon.top + 32;
	rIcon.right = rIcon.left + 32;

	//Start drawing the text 2 pixels after the icon
	rText.left = rIcon.right + 2;
	rText.top = rIcon.top;

	UINT nFormat = DT_LEFT | DT_SINGLELINE | DT_VCENTER;
	if (GetStyle() & LBS_USETABSTOPS)
		nFormat |= DT_EXPANDTABS;

	// If item selected, draw the highlight rectangle.
	// Or if item deselected, draw the rectangle using the window color.
	if ((lpDrawItemStruct->itemState & ODS_SELECTED) && (lpDrawItemStruct->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		CBrush br(::GetSysColor(COLOR_HIGHLIGHT));
		dc->FillRect(&rItem, &br);
	}
	else if (!(lpDrawItemStruct->itemState & ODS_SELECTED) &&
		(lpDrawItemStruct->itemAction & ODA_SELECT))
	{
		CBrush br(dc->GetBkColor());
		//	CBrush br(::GetSysColor(COLOR_WINDOW));
		dc->FillRect(&rItem, &br);
	}

	// If the item has focus, draw the focus rect.
	// If the item does not have focus, erase the focus rect.
	if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && (lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		dc->DrawFocusRect(&rItem);
	}
	else if ((lpDrawItemStruct->itemAction & ODA_FOCUS) && !(lpDrawItemStruct->itemState & ODS_FOCUS))
	{
		dc->DrawFocusRect(&rItem);
	}

	// To draw the Text set the background mode to Transparent.
	int iBkMode = dc->SetBkMode(TRANSPARENT);


	DropItem* data = (DropItem*) this->GetItemDataPtr((int) lpDrawItemStruct->itemID);


	CPoint pt(rIcon.left,rIcon.top);
	//	icons.Draw(dc,0,pt,ILD_NORMAL);
	dc->DrawIcon(pt.x,pt.y,data->icon);




	dc->TextOut(rText.left,rText.top,strText);
	CSize cs = dc->GetTextExtent(strText);

	int oldColor = dc->GetTextColor();
	CFont* oldFont = dc->GetCurrentFont();

	dc->SelectObject(m_FontSmall);
	dc->SetTextColor(m_FontSmallRGB);
	dc->TextOut(rText.left,rText.top + cs.cy + 1,data->lesspath);

	dc->SelectObject(oldFont);
	dc->SetTextColor(oldColor);


}


void SmartComboBox::OnPaint()
{
	// TODO: Add your message handler code here
	// Do not call CComboBox::OnPaint() for painting messages

	if (m_RemoveFrame) {
		CPaintDC dc(this); // device context for painting
		RECT         rect;

		// Find coordinates of client area
		GetClientRect(&rect);


		// Deflate the rectangle by the size of the borders
		InflateRect(&rect, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE));

		// Remove the drop-down button as well
		rect.right -= GetSystemMetrics(SM_CXHSCROLL);

		// Make a mask from the rectangle, so the borders aren't included
		dc.IntersectClipRect(rect.left, rect.top, rect.right, rect.bottom);

		// Draw the combo-box into our DC
		CComboBox::OnPaint();

		// Remove the clipping region
		dc.SelectClipRgn(NULL);

		// now mask off the inside of the control so we can paint the borders
		dc.ExcludeClipRect(rect.left, rect.top, rect.right, rect.bottom);

		// paint a flat colour
		GetClientRect(&rect);
		CBrush b;
		b.CreateStockObject(HOLLOW_BRUSH);
		//b.CreateSolidBrush(RGB(124,124,124));
		dc.FillRect(&rect, &b);
	}
	else {
		CComboBox::OnPaint();
	}
}

void SmartComboBox::OnCbnCloseup()
{
	CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
	if (pDlg == NULL) return;
//	SetCurSel(-1);
//	SetEditSel(searchTxt.GetLength(), searchTxt.GetLength());

	// I don't know why, but if this reformatdisplay isn't here 
	// as well as below, I get issues with the displayed searchTxt
	// being replaced by what's selected as the dropdown closes
	ReformatDisplay();
	ParseSearchTxt();

	int sel = this->GetCurSel();
	if (sel != LB_ERR) {
		DropItem* data = (DropItem*) GetItemDataPtr(sel);

		ReformatDisplay();
		ParseSearchTxt();

		pDlg->smarts->Update(searchTxt, true, data->longpath);
	}

	return;

}

/*
void SmartComboBox::OnDrawSelchange(int itemID) {
	if (!IsWindow(m_listbox.m_hWnd)) return;
	if (!IsWindow(m_edit.m_hWnd)) return;
	// If it's closing, we've already taken care of this..
	if (GetDroppedState()) {
		CLaunchyDlg* pDlg = (CLaunchyDlg*) AfxGetMainWnd();
		m_listbox.GetText(itemID, searchTxt);
		ParseSearchTxt();

		pDlg->smarts->Update(searchTxt,false);
	}

}
*/
void SmartComboBox::PreSubclassWindow()
{
	// TODO: Add your specialized code here and/or call the base class
	//m_edit.SubclassDlgItem(1001, this);
	CComboBox::PreSubclassWindow();
}
