// TextStyleMgrDlg.cpp : 主对话框实现文件
//

#include "stdafx.h"
#include "TextStyleMgrDlg.h"
#include <afxdialogex.h>

// 简单输入对话框（用于重命名功能）
class CInputDialog : public CDialogEx
{
    DECLARE_DYNAMIC(CInputDialog)
public:
    CInputDialog(const CString& title, const CString& label, const CString& initial, CWnd* pParent = NULL)
        : CDialogEx(IDD_TEXTSTYLE_MGR_DIALOG, pParent), m_title(title), m_label(label), m_value(initial) {}
    CString GetInput() const { return m_value; }
protected:
    virtual void DoDataExchange(CDataExchange* pDX) {
        CDialogEx::DoDataExchange(pDX);
        DDX_Text(pDX, IDC_NEW_STYLE_NAME, m_value);
    }
    virtual BOOL OnInitDialog() {
        CDialogEx::OnInitDialog();
        SetWindowText(m_title);
        return TRUE;
    }
private:
    CString m_title, m_label, m_value;
};
IMPLEMENT_DYNAMIC(CInputDialog, CDialogEx)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CTextStyleMgrDlg, CAcUiDialog)

CTextStyleMgrDlg::CTextStyleMgrDlg(CWnd* pParent /*=NULL*/)
    : CAcUiDialog(CTextStyleMgrDlg::IDD, pParent)
{
}

CTextStyleMgrDlg::~CTextStyleMgrDlg()
{
}

void CTextStyleMgrDlg::DoDataExchange(CDataExchange* pDX)
{
    CAcUiDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_STYLE_LIST, m_styleList);
    DDX_Control(pDX, IDC_CURRENT_STYLE, m_currentStyleEdit);

    DDX_Control(pDX, IDC_EDIT_STYLE_NAME, m_editStyleName);
    DDX_Control(pDX, IDC_EDIT_FONT_FILE, m_editFontFile);
    DDX_Control(pDX, IDC_EDIT_BIG_FONT, m_editBigFont);
    DDX_Control(pDX, IDC_EDIT_TEXT_HEIGHT, m_editTextHeight);
    DDX_Control(pDX, IDC_EDIT_WIDTH_FACTOR, m_editWidthFactor);

    DDX_Control(pDX, IDC_BATCH_STYLE_LIST, m_batchStyleList);
    DDX_Control(pDX, IDC_EDIT_KEYWORD, m_editKeyword);
    DDX_Control(pDX, IDC_CHECK_FUZZY, m_checkFuzzy);
    DDX_Control(pDX, IDC_EDIT_BATCH_FONT, m_editBatchFont);
    DDX_Control(pDX, IDC_EDIT_BATCH_BIGFONT, m_editBatchBigFont);

    DDX_Control(pDX, IDC_COMBO_TARGET_STYLE, m_comboTargetStyle);

    DDX_Control(pDX, IDC_EDIT_MISSING_FONT, m_editMissingFont);
    DDX_Control(pDX, IDC_EDIT_MISSING_BIGFONT, m_editMissingBigFont);
    DDX_Control(pDX, IDC_EDIT_MISSING_TTF, m_editMissingTtf);
    DDX_Control(pDX, IDC_COMBO_FONT_CHOICE, m_comboFontChoice);
    DDX_Control(pDX, IDC_CHECK_REMEMBER, m_checkRemember);
    DDX_Control(pDX, IDC_CHECK_AUTO_EXECUTE, m_checkAutoExecute);
}

BEGIN_MESSAGE_MAP(CTextStyleMgrDlg, CAcUiDialog)
    ON_LBN_SELCHANGE(IDC_STYLE_LIST, &CTextStyleMgrDlg::OnLbnSelchangeStyleList)
    ON_BN_CLICKED(IDC_BTN_SET_CURRENT, &CTextStyleMgrDlg::OnBnClickedSetCurrent)
    ON_BN_CLICKED(IDC_BTN_NEW_STYLE, &CTextStyleMgrDlg::OnBnClickedNewStyle)
    ON_BN_CLICKED(IDC_BTN_RENAME_STYLE, &CTextStyleMgrDlg::OnBnClickedRenameStyle)
    ON_BN_CLICKED(IDC_BTN_DELETE_STYLE, &CTextStyleMgrDlg::OnBnClickedDeleteStyle)
    ON_BN_CLICKED(IDC_BTN_CLEAN_STYLE, &CTextStyleMgrDlg::OnBnClickedCleanStyle)
    ON_BN_CLICKED(IDC_BTN_MODIFY_STYLE, &CTextStyleMgrDlg::OnBnClickedModifyStyle)

    ON_BN_CLICKED(IDC_BTN_BATCH_SELECT_ALL, &CTextStyleMgrDlg::OnBnClickedBatchSelectAll)
    ON_BN_CLICKED(IDC_BTN_BATCH_CLEAR, &CTextStyleMgrDlg::OnBnClickedBatchClear)
    ON_BN_CLICKED(IDC_BTN_BATCH_REPLACE, &CTextStyleMgrDlg::OnBnClickedBatchReplace)

    ON_BN_CLICKED(IDC_BTN_UNIFY_STYLE, &CTextStyleMgrDlg::OnBnClickedUnifyStyle)
    ON_BN_CLICKED(IDC_BTN_UNIFY_SELECTION, &CTextStyleMgrDlg::OnBnClickedUnifySelection)

    ON_BN_CLICKED(IDC_BTN_REPLACE_MISSING, &CTextStyleMgrDlg::OnBnClickedReplaceMissing)
    ON_CBN_SELCHANGE(IDC_COMBO_FONT_CHOICE, &CTextStyleMgrDlg::OnCbnSelchangeFontChoice)

    ON_BN_CLICKED(IDC_BTN_CLOSE, &CTextStyleMgrDlg::OnBnClickedClose)
    ON_BN_CLICKED(IDC_BTN_HELP, &CTextStyleMgrDlg::OnBnClickedHelp)
    ON_BN_CLICKED(IDC_BTN_MORE_PLUGINS, &CTextStyleMgrDlg::OnBnClickedMorePlugins)
END_MESSAGE_MAP()

BOOL CTextStyleMgrDlg::OnInitDialog()
{
    CAcUiDialog::OnInitDialog();

    // 设置默认值
    m_editBatchFont.SetWindowText(_T("gbnor.shx"));
    m_editBatchBigFont.SetWindowText(_T("gbcbig.shx"));
    m_editMissingFont.SetWindowText(_T("gbnor.shx"));
    m_editMissingBigFont.SetWindowText(_T("gbcbig.shx"));
    m_editMissingTtf.SetWindowText(_T("仿宋"));

    // 初始化常用字体下拉框
    m_comboFontChoice.AddString(_T("使用国标字体"));
    m_comboFontChoice.AddString(_T("宋体"));
    m_comboFontChoice.AddString(_T("黑体"));
    m_comboFontChoice.AddString(_T("楷体"));
    m_comboFontChoice.AddString(_T("仿宋"));
    m_comboFontChoice.AddString(_T("微软雅黑"));
    m_comboFontChoice.AddString(_T("微软雅黑粗体"));
    m_comboFontChoice.AddString(_T("隶书"));
    m_comboFontChoice.AddString(_T("幼圆"));
    m_comboFontChoice.SetCurSel(0);

    // 刷新样式列表
    RefreshStyleList();

    return TRUE;
}

void CTextStyleMgrDlg::RefreshStyleList()
{
    m_styleMgr.GetAllStyles(m_styles);

    // 刷新左侧样式列表
    m_styleList.ResetContent();
    for (size_t i = 0; i < m_styles.size(); i++)
    {
        m_styleList.AddString(m_styles[i].name);
    }

    // 刷新批量替换样式列表
    m_batchStyleList.ResetContent();
    for (size_t i = 0; i < m_styles.size(); i++)
    {
        m_batchStyleList.AddString(m_styles[i].name);
    }

    // 刷新目标样式下拉框
    m_comboTargetStyle.ResetContent();
    for (size_t i = 0; i < m_styles.size(); i++)
    {
        m_comboTargetStyle.AddString(m_styles[i].name);
    }
    if (m_styles.size() > 0)
        m_comboTargetStyle.SetCurSel(0);

    // 更新当前样式显示
    UpdateCurrentStyleDisplay();
}

void CTextStyleMgrDlg::UpdateCurrentStyleDisplay()
{
    CString currentStyle = m_styleMgr.GetCurrentStyleName();
    m_currentStyleEdit.SetWindowText(currentStyle);
}

void CTextStyleMgrDlg::ClearInfoFields()
{
    m_editStyleName.SetWindowText(_T(""));
    m_editFontFile.SetWindowText(_T(""));
    m_editBigFont.SetWindowText(_T("无"));
    m_editTextHeight.SetWindowText(_T("0"));
    m_editWidthFactor.SetWindowText(_T("1"));
}

void CTextStyleMgrDlg::OnLbnSelchangeStyleList()
{
    int sel = m_styleList.GetCurSel();
    if (sel == LB_ERR || sel >= (int)m_styles.size())
    {
        ClearInfoFields();
        return;
    }

    const TextStyleInfo& info = m_styles[sel];
    m_editStyleName.SetWindowText(info.name);
    m_editFontFile.SetWindowText(info.fontFile.IsEmpty() ? _T("") : info.fontFile);
    m_editBigFont.SetWindowText(info.bigFontFile.IsEmpty() ? _T("无") : info.bigFontFile);

    // 同步批量替换列表框的选中项
    if (sel >= 0 && sel < m_batchStyleList.GetCount())
    {
        m_batchStyleList.SetSel(sel, TRUE);
    }

    CString strTemp;
    strTemp.Format(_T("%.4g"), info.textSize);
    m_editTextHeight.SetWindowText(strTemp);

    strTemp.Format(_T("%.4g"), info.widthFactor);
    m_editWidthFactor.SetWindowText(strTemp);
}

void CTextStyleMgrDlg::OnBnClickedSetCurrent()
{
    int sel = m_styleList.GetCurSel();
    if (sel == LB_ERR)
    {
        AfxMessageBox(_T("请先在左侧选择一个样式"));
        return;
    }

    CString styleName;
    m_styleList.GetText(sel, styleName);

    if (m_styleMgr.SetCurrentStyle(styleName))
    {
        UpdateCurrentStyleDisplay();
        AfxMessageBox(_T("已设置当前样式为: ") + styleName, MB_ICONINFORMATION);
    }
    else
    {
        AfxMessageBox(_T("设置当前样式失败"), MB_ICONERROR);
    }
}

void CTextStyleMgrDlg::OnBnClickedNewStyle()
{
    CDialogEx dlg(IDD_NEW_STYLE_DIALOG, this);
    if (dlg.DoModal() == IDOK)
    {
        CString name, fontFile, bigFont, height, widthFactor;
        dlg.GetDlgItemText(IDC_NEW_STYLE_NAME, name);
        dlg.GetDlgItemText(IDC_NEW_FONT_FILE, fontFile);
        dlg.GetDlgItemText(IDC_NEW_BIG_FONT, bigFont);
        dlg.GetDlgItemText(IDC_NEW_TEXT_HEIGHT, height);
        dlg.GetDlgItemText(IDC_NEW_WIDTH_FACTOR, widthFactor);

        if (name.IsEmpty())
        {
            AfxMessageBox(_T("请输入样式名称"));
            return;
        }

        CString errMsg;
        if (m_styleMgr.CreateStyle(name, fontFile, bigFont,
                                   _tstof(height), _tstof(widthFactor), errMsg))
        {
            RefreshStyleList();
            AfxMessageBox(_T("样式创建成功: ") + name, MB_ICONINFORMATION);
        }
        else
        {
            AfxMessageBox(_T("创建失败: ") + errMsg, MB_ICONERROR);
        }
    }
}

void CTextStyleMgrDlg::OnBnClickedRenameStyle()
{
    int sel = m_styleList.GetCurSel();
    if (sel == LB_ERR)
    {
        AfxMessageBox(_T("请先选择一个样式"));
        return;
    }

    CString oldName;
    m_styleList.GetText(sel, oldName);

    if (oldName.CompareNoCase(_T("Standard")) == 0)
    {
        AfxMessageBox(_T("不能重命名Standard样式"));
        return;
    }

    // 弹出输入对话框
    CInputDialog inputDlg(_T("重命名样式"), _T("请输入新名称:"), oldName, this);
    if (inputDlg.DoModal() == IDOK)
    {
        CString newName = inputDlg.GetInput();
        if (newName.IsEmpty() || newName == oldName)
            return;

        CString errMsg;
        if (m_styleMgr.RenameStyle(oldName, newName, errMsg))
        {
            RefreshStyleList();
            AfxMessageBox(_T("重命名成功"), MB_ICONINFORMATION);
        }
        else
        {
            AfxMessageBox(_T("重命名失败: ") + errMsg, MB_ICONERROR);
        }
    }
}

void CTextStyleMgrDlg::OnBnClickedDeleteStyle()
{
    int sel = m_styleList.GetCurSel();
    if (sel == LB_ERR)
    {
        AfxMessageBox(_T("请先选择一个样式"));
        return;
    }

    CString styleName;
    m_styleList.GetText(sel, styleName);

    if (styleName.CompareNoCase(_T("Standard")) == 0)
    {
        AfxMessageBox(_T("不能删除Standard样式"));
        return;
    }

    if (AfxMessageBox(_T("确定要删除样式 '") + styleName + _T("' 吗？"),
                      MB_YESNO | MB_ICONQUESTION) != IDYES)
        return;

    CString errMsg;
    if (m_styleMgr.DeleteStyle(styleName, errMsg))
    {
        RefreshStyleList();
        ClearInfoFields();
        AfxMessageBox(_T("样式已删除: ") + styleName, MB_ICONINFORMATION);
    }
    else
    {
        AfxMessageBox(_T("删除失败: ") + errMsg, MB_ICONERROR);
    }
}

void CTextStyleMgrDlg::OnBnClickedCleanStyle()
{
    if (AfxMessageBox(_T("将删除所有未使用的文字样式（除Standard外），是否继续？"),
                      MB_YESNO | MB_ICONQUESTION) != IDYES)
        return;

    CString errMsg;
    int count = m_styleMgr.CleanUnusedStyles(errMsg);
    RefreshStyleList();

    CString msg;
    msg.Format(_T("清理完成，共删除 %d 个未使用样式"), count);
    AfxMessageBox(msg, MB_ICONINFORMATION);
}

void CTextStyleMgrDlg::OnBnClickedModifyStyle()
{
    int sel = m_styleList.GetCurSel();
    if (sel == LB_ERR || sel >= (int)m_styles.size())
    {
        AfxMessageBox(_T("请先选择一个样式"));
        return;
    }

    CString styleName = m_styles[sel].name;

    CString fontFile, bigFont, height, widthFactor;
    m_editFontFile.GetWindowText(fontFile);
    m_editBigFont.GetWindowText(bigFont);
    m_editTextHeight.GetWindowText(height);
    m_editWidthFactor.GetWindowText(widthFactor);

    if (bigFont.CompareNoCase(_T("无")) == 0)
        bigFont = _T("");

    CString errMsg;
    double hVal = _tstof(height);
    double wfVal = _tstof(widthFactor);
    if (hVal < 0) hVal = 0;
    if (wfVal <= 0) wfVal = 1.0;
    if (m_styleMgr.UpdateStyleInfo(styleName, fontFile, bigFont,
                                   hVal, wfVal, errMsg))
    {
        RefreshStyleList();
        AfxMessageBox(_T("样式信息已更新"), MB_ICONINFORMATION);
    }
    else
    {
        AfxMessageBox(_T("更新失败: ") + errMsg, MB_ICONERROR);
    }
}

int CTextStyleMgrDlg::GetSelectedStylesFromListBox(CListBox& listBox, std::vector<CString>& styles)
{
    styles.clear();
    int count = listBox.GetSelCount();
    if (count == LB_ERR || count == 0)
        return 0;

    int* indices = new int[count];
    listBox.GetSelItems(count, indices);

    for (int i = 0; i < count; i++)
    {
        CString text;
        listBox.GetText(indices[i], text);
        styles.push_back(text);
    }

    delete[] indices;
    return count;
}

void CTextStyleMgrDlg::OnBnClickedBatchSelectAll()
{
    int count = m_batchStyleList.GetCount();
    for (int i = 0; i < count; i++)
        m_batchStyleList.SetSel(i, TRUE);
}

void CTextStyleMgrDlg::OnBnClickedBatchClear()
{
    m_batchStyleList.SelItemRange(FALSE, 0, m_batchStyleList.GetCount() - 1);
}

void CTextStyleMgrDlg::OnBnClickedBatchReplace()
{
    std::vector<CString> selectedStyles;
    GetSelectedStylesFromListBox(m_batchStyleList, selectedStyles);

    CString keyword;
    m_editKeyword.GetWindowText(keyword);
    keyword.Trim();

    bool fuzzy = (m_checkFuzzy.GetCheck() == BST_CHECKED);

    CString fontFile, bigFont;
    m_editBatchFont.GetWindowText(fontFile);
    m_editBatchBigFont.GetWindowText(bigFont);

    if (selectedStyles.empty() && keyword.IsEmpty())
    {
        AfxMessageBox(_T("请选择样式或输入关键字"));
        return;
    }

    if (fontFile.IsEmpty() && bigFont.IsEmpty())
    {
        AfxMessageBox(_T("请设置要替换的字体"));
        return;
    }

    CString errMsg;
    int count = m_styleMgr.BatchReplaceFont(selectedStyles, keyword, fuzzy,
                                            fontFile, bigFont, errMsg);
    RefreshStyleList();

    CString msg;
    msg.Format(_T("批量替换完成，共更新 %d 个样式"), count);
    AfxMessageBox(msg, MB_ICONINFORMATION);
}

void CTextStyleMgrDlg::OnBnClickedUnifyStyle()
{
    CString targetStyle;
    m_comboTargetStyle.GetWindowText(targetStyle);
    if (targetStyle.IsEmpty())
    {
        AfxMessageBox(_T("请选择目标样式"));
        return;
    }

    int affectedCount = 0;
    CString errMsg;
    if (m_styleMgr.UnifyTextStyles(targetStyle, false, affectedCount, errMsg))
    {
        CString msg;
        msg.Format(_T("已将 %d 个文字对象统一为样式 '%s'"), affectedCount, targetStyle);
        AfxMessageBox(msg, MB_ICONINFORMATION);
    }
    else
    {
        AfxMessageBox(_T("统一样式失败: ") + errMsg, MB_ICONERROR);
    }
}

void CTextStyleMgrDlg::OnBnClickedUnifySelection()
{
    CString targetStyle;
    m_comboTargetStyle.GetWindowText(targetStyle);
    if (targetStyle.IsEmpty())
    {
        AfxMessageBox(_T("请先选择目标样式"));
        return;
    }

    // 隐藏对话框让用户选择对象
    ShowWindow(SW_HIDE);
    acutPrintf(_T("\n请在CAD中选择要统一为 '%s' 的文字对象..."), targetStyle);

    int affectedCount = 0;
    CString errMsg;
    bool success = m_styleMgr.UnifyTextStyles(targetStyle, true, affectedCount, errMsg);

    ShowWindow(SW_SHOW);

    if (success)
    {
        CString msg;
        msg.Format(_T("已将 %d 个文字对象统一为样式 '%s'"), affectedCount, targetStyle);
        AfxMessageBox(msg, MB_ICONINFORMATION);
    }
    else
    {
        AfxMessageBox(_T("统一样式失败: ") + errMsg, MB_ICONERROR);
    }
}

void CTextStyleMgrDlg::OnCbnSelchangeFontChoice()
{
    int sel = m_comboFontChoice.GetCurSel();
    if (sel == CB_ERR)
        return;

    CString choice;
    m_comboFontChoice.GetLBText(sel, choice);

    // 根据选择设置字体
    if (choice == _T("使用国标字体"))
    {
        m_editMissingFont.SetWindowText(_T("gbnor.shx"));
        m_editMissingBigFont.SetWindowText(_T("gbcbig.shx"));
        m_editMissingTtf.SetWindowText(_T("仿宋"));
    }
    else if (choice == _T("宋体"))
    {
        m_editMissingFont.SetWindowText(_T("simsun.ttc"));
        m_editMissingBigFont.SetWindowText(_T("gbcbig.shx"));
        m_editMissingTtf.SetWindowText(_T("SimSun"));
    }
    else if (choice == _T("黑体"))
    {
        m_editMissingFont.SetWindowText(_T("simhei.ttf"));
        m_editMissingBigFont.SetWindowText(_T("gbcbig.shx"));
        m_editMissingTtf.SetWindowText(_T("SimHei"));
    }
    else if (choice == _T("楷体"))
    {
        m_editMissingFont.SetWindowText(_T("simkai.ttf"));
        m_editMissingBigFont.SetWindowText(_T("gbcbig.shx"));
        m_editMissingTtf.SetWindowText(_T("KaiTi"));
    }
    else if (choice == _T("仿宋"))
    {
        m_editMissingFont.SetWindowText(_T("simfang.ttf"));
        m_editMissingBigFont.SetWindowText(_T("gbcbig.shx"));
        m_editMissingTtf.SetWindowText(_T("FangSong"));
    }
    else if (choice == _T("微软雅黑"))
    {
        m_editMissingFont.SetWindowText(_T("msyh.ttc"));
        m_editMissingBigFont.SetWindowText(_T("gbcbig.shx"));
        m_editMissingTtf.SetWindowText(_T("Microsoft YaHei"));
    }
    else if (choice == _T("微软雅黑粗体"))
    {
        m_editMissingFont.SetWindowText(_T("msyhbd.ttc"));
        m_editMissingBigFont.SetWindowText(_T("gbcbig.shx"));
        m_editMissingTtf.SetWindowText(_T("Microsoft YaHei Bold"));
    }
    else if (choice == _T("隶书"))
    {
        m_editMissingFont.SetWindowText(_T("simli.ttf"));
        m_editMissingBigFont.SetWindowText(_T("gbcbig.shx"));
        m_editMissingTtf.SetWindowText(_T("LiSu"));
    }
    else if (choice == _T("幼圆"))
    {
        m_editMissingFont.SetWindowText(_T("simyou.ttf"));
        m_editMissingBigFont.SetWindowText(_T("gbcbig.shx"));
        m_editMissingTtf.SetWindowText(_T("YouYuan"));
    }
}

void CTextStyleMgrDlg::OnBnClickedReplaceMissing()
{
    CString defaultFont, defaultBigFont, defaultTtf;
    m_editMissingFont.GetWindowText(defaultFont);
    m_editMissingBigFont.GetWindowText(defaultBigFont);
    m_editMissingTtf.GetWindowText(defaultTtf);

    if (defaultFont.IsEmpty())
    {
        AfxMessageBox(_T("请设置默认字体"));
        return;
    }

    CString errMsg;
    int count = m_styleMgr.ReplaceMissingFonts(defaultFont, defaultBigFont, defaultTtf, errMsg);
    RefreshStyleList();

    if (count == 0)
    {
        AfxMessageBox(_T("没有发现缺失字体"), MB_ICONINFORMATION);
    }
    else
    {
        CString msg;
        msg.Format(_T("已替换 %d 个缺失字体"), count);
        AfxMessageBox(msg, MB_ICONINFORMATION);
    }
}

void CTextStyleMgrDlg::OnBnClickedClose()
{
    OnOK();
}

void CTextStyleMgrDlg::OnBnClickedHelp()
{
    CString helpText;
    helpText = _T("文字样式与字体管理工具 V4.0 by蒋先生\r\n\r\n")
        _T("【功能说明】\r\n")
        _T("1. 文字样式列表管理\r\n")
        _T("   - 新建：创建新的文字样式\r\n")
        _T("   - 重命名：修改样式名称\r\n")
        _T("   - 删除：删除选中的样式\r\n")
        _T("   - 清理：删除所有未使用的样式\r\n\r\n")
        _T("2. 样式信息编辑\r\n")
        _T("   - 样式名：修改样式名称\r\n")
        _T("   - 字体：设置主字体文件\r\n")
        _T("   - 大字体：设置大字体文件\r\n")
        _T("   - 字高：设置默认字高（0为自适应）\r\n")
        _T("   - 宽度因子：设置文字宽度比例\r\n\r\n")
        _T("3. 批量替换文字样式字体\r\n")
        _T("   - 可选择多个样式或输入关键字\r\n")
        _T("   - 支持模糊匹配\r\n")
        _T("   - 设置新字体和大字体\r\n\r\n")
        _T("4. 批量统一文字样式\r\n")
        _T("   - 将所有文字或选中文字统一为目标样式\r\n\r\n")
        _T("5. 批量替换缺失字体\r\n")
        _T("   - 自动检测并替换缺失的字体文件\r\n\r\n")
        _T("【使用提示】\r\n")
        _T("- 请确保CAD程序正在运行\r\n")
        _T("- 使用国标字体需确保CAD已安装相应SHX字体\r\n")
        _T("- 自定义字体请确保已安装到系统中\r\n")
        _T("- 命令: TSM 启动本工具");

    ShowHelpDialog(_T("帮助说明"), helpText);
}

void CTextStyleMgrDlg::OnBnClickedMorePlugins()
{
    CString pluginsText;
    pluginsText = _T("更多CAD插件推荐：\r\n\r\n")
        _T("1. CAD图纸批量打印工具\r\n")
        _T("   - 支持批量打印多张图纸\r\n")
        _T("   - 支持自定义打印参数\r\n\r\n")
        _T("2. CAD属性提取工具\r\n")
        _T("   - 提取图块属性到Excel\r\n")
        _T("   - 支持批量导出\r\n\r\n")
        _T("3. CAD标注样式管理工具\r\n")
        _T("   - 管理标注样式\r\n")
        _T("   - 批量修改标注设置\r\n\r\n")
        _T("4. CAD图纸版本对比工具\r\n")
        _T("   - 对比两个版本的图纸\r\n")
        _T("   - 高亮显示差异区域\r\n\r\n")
        _T("5. CAD图层管理工具\r\n")
        _T("   - 批量管理图层\r\n")
        _T("   - 图层状态保存与恢复");

    ShowHelpDialog(_T("更多插件"), pluginsText);
}

void CTextStyleMgrDlg::ShowHelpDialog(const CString& title, const CString& content)
{
    CHelpDlg helpDlg(this);
    helpDlg.m_helpContent = content;
    helpDlg.m_helpTitle = title;
    helpDlg.DoModal();
}
