// StyleManager.h : 文字样式管理核心逻辑
//

#pragma once

#include "stdafx.h"
#include <aced.h>
#include <rxregsvc.h>
#include <dbapserv.h>
#include <dbsymtb.h>
#include <dbents.h>
#include <AcApDMgr.h>
#include <vector>
#include <set>

// 文字样式信息结构体
struct TextStyleInfo
{
    CString     name;           // 样式名
    CString     fontFile;       // 字体文件
    CString     bigFontFile;    // 大字体文件
    double      textSize;       // 字高
    double      widthFactor;    // 宽度因子
    double      obliqueAngle;   // 倾斜角度
    bool        isCurrent;      // 是否当前样式
};

class CStyleManager
{
public:
    CStyleManager();
    ~CStyleManager();

    // 获取所有文字样式
    bool GetAllStyles(std::vector<TextStyleInfo>& styles);

    // 获取当前文字样式名
    CString GetCurrentStyleName();

    // 设置当前文字样式
    bool SetCurrentStyle(const CString& styleName);

    // 创建新样式
    bool CreateStyle(const CString& name, const CString& fontFile,
                     const CString& bigFont, double height, double widthFactor,
                     CString& errMsg);

    // 重命名样式
    bool RenameStyle(const CString& oldName, const CString& newName,
                     CString& errMsg);

    // 删除样式
    bool DeleteStyle(const CString& styleName, CString& errMsg);

    // 更新样式信息
    bool UpdateStyleInfo(const CString& styleName, const CString& fontFile,
                         const CString& bigFont, double height, double widthFactor,
                         CString& errMsg);

    // 清理未使用的样式
    int CleanUnusedStyles(CString& errMsg);

    // 批量替换字体
    int BatchReplaceFont(const std::vector<CString>& styleNames,
                         const CString& keyword, bool fuzzyMatch,
                         const CString& fontFile, const CString& bigFont,
                         CString& errMsg);

    // 批量统一文字样式
    bool UnifyTextStyles(const CString& targetStyle, bool useSelection,
                         int& affectedCount, CString& errMsg);

    // 替换缺失字体
    int ReplaceMissingFonts(const CString& defaultFont,
                            const CString& defaultBigFont,
                            const CString& defaultTtf,
                            CString& errMsg);

    // 检查字体文件是否存在
    static bool IsFontExists(const CString& fontName);

private:
    // 获取当前数据库
    AcDbDatabase* GetWorkingDatabase();

    // 遍历获取已使用的样式名集合
    void GetUsedStyleNames(std::set<CString>& usedNames);

    // 遍历模型空间和图纸空间对象
    template<typename Func>
    void IterateSpaceObjects(AcDbDatabase* pDb, Func func);
};
