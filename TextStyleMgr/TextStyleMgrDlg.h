// TextStyleMgrDlg.h : 主对话框头文件
//

#pragma once

#include "resource.h"
#include <dbsymtb.h>
#include <dbapserv.h>
#include <adslib.h>
#include <adui.h>
#include <acui.h>
#include "StyleManager.h"

class CTextStyleMgrDlg : public CAcUiDialog
{
    DECLARE_DYNAMIC(CTextStyleMgrDlg)

public:
    CTextStyleMgrDlg(CWnd* pParent = NULL);
    virtual ~CTextStyleMgrDlg();

    enum { IDD = IDD_TEXTSTYLE_MGR_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

private:
    CStyleManager m_styleMgr;
    std::vector<TextStyleInfo> m_styles;

    // 左侧 - 样式列表
    CListBox m_styleList;
    CEdit   m_currentStyleEdit;

    // 右侧 - 样式信息编辑
    CEdit   m_editStyleName;
    CEdit   m_editFontFile;
    CEdit   m_editBigFont;
    CEdit   m_editTextHeight;
    CEdit   m_editWidthFactor;

    // 批量替换区域
    CListBox m_batchStyleList;
    CEdit   m_editKeyword;
    CButton m_checkFuzzy;
    CEdit   m_editBatchFont;
    CEdit   m_editBatchBigFont;

    // 批量统一样式区域
    CComboBox m_comboTargetStyle;

    // 替换缺失字体区域
    CEdit   m_editMissingFont;
    CEdit   m_editMissingBigFont;
    CEdit   m_editMissingTtf;
    CComboBox m_comboFontChoice;
    CButton m_checkRemember;
    CButton m_checkAutoExecute;

    // 内部方法
    void RefreshStyleList();
    void RefreshBatchStyleList();
    void RefreshTargetStyleCombo();
    void UpdateCurrentStyleDisplay();
    void ClearInfoFields();
    void LoadStyleInfoToListBox(CListBox& listBox);
    int  GetSelectedStylesFromListBox(CListBox& listBox, std::vector<CString>& styles);
    void ShowHelpDialog(const CString& title, const CString& content);

    // 消息处理函数
    afx_msg void OnLbnSelchangeStyleList();
    afx_msg void OnBnClickedSetCurrent();
    afx_msg void OnBnClickedNewStyle();
    afx_msg void OnBnClickedRenameStyle();
    afx_msg void OnBnClickedDeleteStyle();
    afx_msg void OnBnClickedCleanStyle();
    afx_msg void OnBnClickedModifyStyle();

    afx_msg void OnBnClickedBatchSelectAll();
    afx_msg void OnBnClickedBatchClear();
    afx_msg void OnBnClickedBatchReplace();

    afx_msg void OnBnClickedUnifyStyle();
    afx_msg void OnBnClickedUnifySelection();

    afx_msg void OnBnClickedReplaceMissing();
    afx_msg void OnCbnSelchangeFontChoice();

    afx_msg void OnBnClickedClose();
    afx_msg void OnBnClickedHelp();
};
