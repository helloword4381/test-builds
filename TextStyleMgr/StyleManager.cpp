// StyleManager.cpp : 文字样式管理核心逻辑实现
//

#include "stdafx.h"
#include "StyleManager.h"
#include <set>
#include <shlwapi.h>

CStyleManager::CStyleManager()
{
}

CStyleManager::~CStyleManager()
{
}

AcDbDatabase* CStyleManager::GetWorkingDatabase()
{
    return acdbHostApplicationServices()->workingDatabase();
}

bool CStyleManager::GetAllStyles(std::vector<TextStyleInfo>& styles)
{
    styles.clear();

    AcDbDatabase* pDb = GetWorkingDatabase();
    if (!pDb)
        return false;

    AcDbTextStyleTable* pTextStyleTable = NULL;
    Acad::ErrorStatus es = pDb->getTextStyleTable(pTextStyleTable, AcDb::kForRead);
    if (es != Acad::eOk)
        return false;

    // 获取当前样式名
    CString currentStyle = GetCurrentStyleName();

    AcDbTextStyleTableIterator* pIter = NULL;
    es = pTextStyleTable->newIterator(pIter);
    if (es != Acad::eOk)
    {
        pTextStyleTable->close();
        return false;
    }

    for (pIter->start(); !pIter->done(); pIter->step())
    {
        AcDbTextStyleTableRecord* pRecord = NULL;
        es = pIter->getRecord(pRecord, AcDb::kForRead);
        if (es != Acad::eOk)
            continue;

        TextStyleInfo info;
        AcString strName;
        pRecord->getName(strName);
        info.name = strName.kTCharPtr();

        AcString strFileName;
        pRecord->fileName(strFileName);
        info.fontFile = strFileName.kTCharPtr();

        AcString strBigFont;
        pRecord->bigFontFileName(strBigFont);
        info.bigFontFile = strBigFont.kTCharPtr();

        info.textSize = pRecord->textSize();
        info.widthFactor = pRecord->xScale();
        info.obliqueAngle = pRecord->obliquingAngle();
        info.isCurrent = (info.name.CompareNoCase(currentStyle) == 0);

        styles.push_back(info);
        pRecord->close();
    }

    delete pIter;
    pTextStyleTable->close();
    return true;
}

CString CStyleManager::GetCurrentStyleName()
{
    AcDbDatabase* pDb = GetWorkingDatabase();
    if (!pDb)
        return _T("Standard");

    AcDbObjectId tsId = pDb->textstyle();
    AcDbTextStyleTableRecord* pRecord = NULL;
    Acad::ErrorStatus es = acdbOpenObject(pRecord, tsId, AcDb::kForRead);
    if (es != Acad::eOk)
        return _T("Standard");

    AcString strName;
    pRecord->getName(strName);
    CString result = strName.kTCharPtr();
    pRecord->close();
    return result;
}

bool CStyleManager::SetCurrentStyle(const CString& styleName)
{
    AcDbDatabase* pDb = GetWorkingDatabase();
    if (!pDb)
        return false;

    AcDbTextStyleTable* pTable = NULL;
    Acad::ErrorStatus es = pDb->getTextStyleTable(pTable, AcDb::kForRead);
    if (es != Acad::eOk)
        return false;

    AcDbTextStyleTableRecord* pRecord = NULL;
    es = pTable->getAt(styleName, pRecord, AcDb::kForRead);
    pTable->close();
    if (es != Acad::eOk)
        return false;

    AcDbObjectId styleId = pRecord->objectId();
    pRecord->close();

    pDb->setTextstyle(styleId);
    return true;
}

bool CStyleManager::CreateStyle(const CString& name, const CString& fontFile,
                                const CString& bigFont, double height, double widthFactor,
                                CString& errMsg)
{
    AcDbDatabase* pDb = GetWorkingDatabase();
    if (!pDb)
    {
        errMsg = _T("无法获取数据库");
        return false;
    }

    AcDbTextStyleTable* pTable = NULL;
    Acad::ErrorStatus es = pDb->getTextStyleTable(pTable, AcDb::kForRead);
    if (es != Acad::eOk)
    {
        errMsg = _T("无法打开文字样式表");
        return false;
    }

    // 检查是否已存在
    if (pTable->has(name))
    {
        pTable->close();
        errMsg = _T("样式已存在");
        return false;
    }

    AcDbTextStyleTableRecord* pNewRecord = new AcDbTextStyleTableRecord();
    pNewRecord->setName(name);

    if (!fontFile.IsEmpty())
        pNewRecord->setFileName(fontFile);
    if (!bigFont.IsEmpty())
        pNewRecord->setBigFontFileName(bigFont);
    if (height > 0)
        pNewRecord->setTextSize(height);
    if (widthFactor > 0)
        pNewRecord->setXScale(widthFactor);

    es = pTable->upgradeOpen();
    if (es != Acad::eOk)
    {
        delete pNewRecord;
        pTable->close();
        errMsg = _T("无法升级样式表权限");
        return false;
    }

    es = pTable->add(pNewRecord);
    pTable->close();

    if (es != Acad::eOk)
    {
        delete pNewRecord;
        errMsg = _T("添加样式失败");
        return false;
    }

    pNewRecord->close();
    return true;
}

bool CStyleManager::RenameStyle(const CString& oldName, const CString& newName,
                                CString& errMsg)
{
    if (oldName.CompareNoCase(_T("Standard")) == 0)
    {
        errMsg = _T("不能重命名Standard样式");
        return false;
    }

    AcDbDatabase* pDb = GetWorkingDatabase();
    if (!pDb)
        return false;

    AcDbTextStyleTable* pTable = NULL;
    Acad::ErrorStatus es = pDb->getTextStyleTable(pTable, AcDb::kForRead);
    if (es != Acad::eOk)
        return false;

    AcDbTextStyleTableRecord* pRecord = NULL;
    es = pTable->getAt(oldName, pRecord, AcDb::kForWrite);
    pTable->close();
    if (es != Acad::eOk)
    {
        errMsg = _T("样式不存在");
        return false;
    }

    es = pRecord->setName(newName);
    pRecord->close();

    if (es != Acad::eOk)
    {
        errMsg = _T("重命名失败");
        return false;
    }

    return true;
}

bool CStyleManager::DeleteStyle(const CString& styleName, CString& errMsg)
{
    if (styleName.CompareNoCase(_T("Standard")) == 0)
    {
        errMsg = _T("不能删除Standard样式");
        return false;
    }

    AcDbDatabase* pDb = GetWorkingDatabase();
    if (!pDb)
        return false;

    // 如果是当前样式，先切换到Standard
    CString currentStyle = GetCurrentStyleName();
    if (currentStyle.CompareNoCase(styleName) == 0)
    {
        SetCurrentStyle(_T("Standard"));
    }

    AcDbTextStyleTable* pTable = NULL;
    Acad::ErrorStatus es = pDb->getTextStyleTable(pTable, AcDb::kForRead);
    if (es != Acad::eOk)
        return false;

    AcDbTextStyleTableRecord* pRecord = NULL;
    es = pTable->getAt(styleName, pRecord, AcDb::kForWrite);
    pTable->close();
    if (es != Acad::eOk)
    {
        errMsg = _T("样式不存在或正在使用中");
        return false;
    }

    es = pRecord->erase();
    pRecord->close();

    if (es != Acad::eOk)
    {
        errMsg = _T("删除失败，样式可能正在使用中");
        return false;
    }

    return true;
}

bool CStyleManager::UpdateStyleInfo(const CString& styleName, const CString& fontFile,
                                    const CString& bigFont, double height, double widthFactor,
                                    CString& errMsg)
{
    AcDbDatabase* pDb = GetWorkingDatabase();
    if (!pDb)
        return false;

    AcDbTextStyleTable* pTable = NULL;
    Acad::ErrorStatus es = pDb->getTextStyleTable(pTable, AcDb::kForRead);
    if (es != Acad::eOk)
        return false;

    AcDbTextStyleTableRecord* pRecord = NULL;
    es = pTable->getAt(styleName, pRecord, AcDb::kForWrite);
    pTable->close();
    if (es != Acad::eOk)
    {
        errMsg = _T("样式不存在");
        return false;
    }

    if (!fontFile.IsEmpty())
        pRecord->setFileName(fontFile);
    if (!bigFont.IsEmpty())
        pRecord->setBigFontFileName(bigFont);
    if (height >= 0)
        pRecord->setTextSize(height);
    if (widthFactor > 0)
        pRecord->setXScale(widthFactor);

    pRecord->close();
    return true;
}

void CStyleManager::GetUsedStyleNames(std::set<CString>& usedNames)
{
    AcDbDatabase* pDb = GetWorkingDatabase();
    if (!pDb)
        return;

    // 遍历模型空间
    AcDbBlockTable* pBlockTable = NULL;
    Acad::ErrorStatus es = pDb->getBlockTable(pBlockTable, AcDb::kForRead);
    if (es != Acad::eOk)
        return;

    AcDbBlockTableRecord* pModelSpace = NULL;
    es = pBlockTable->getAt(ACDB_MODEL_SPACE, pModelSpace, AcDb::kForRead);
    pBlockTable->close();
    if (es != Acad::eOk)
        return;

    AcDbBlockTableRecordIterator* pIter = NULL;
    pModelSpace->newIterator(pIter);
    for (pIter->start(); !pIter->done(); pIter->step())
    {
        AcDbEntity* pEnt = NULL;
        if (pIter->getEntity(pEnt, AcDb::kForRead) == Acad::eOk)
        {
            // 检查是否是文字对象
            AcDbText* pText = AcDbText::cast(pEnt);
            AcDbMText* pMText = AcDbMText::cast(pEnt);
            if (pText || pMText)
            {
                AcDbObjectId styleId;
                if (pText)
                    styleId = pText->textStyle();
                else
                    styleId = pMText->textStyle();

                AcDbTextStyleTableRecord* pTsRecord = NULL;
                if (acdbOpenObject(pTsRecord, styleId, AcDb::kForRead) == Acad::eOk)
                {
                    AcString strName;
                    pTsRecord->getName(strName);
                    usedNames.insert(strName.kTCharPtr());
                    pTsRecord->close();
                }
            }
            pEnt->close();
        }
    }
    delete pIter;
    pModelSpace->close();

    // 遍历图纸空间
    es = pDb->getBlockTable(pBlockTable, AcDb::kForRead);
    if (es == Acad::eOk)
    {
        es = pBlockTable->getAt(ACDB_PAPER_SPACE, pModelSpace, AcDb::kForRead);
        pBlockTable->close();
        if (es == Acad::eOk)
        {
            pModelSpace->newIterator(pIter);
            for (pIter->start(); !pIter->done(); pIter->step())
            {
                AcDbEntity* pEnt = NULL;
                if (pIter->getEntity(pEnt, AcDb::kForRead) == Acad::eOk)
                {
                    AcDbText* pText = AcDbText::cast(pEnt);
                    AcDbMText* pMText = AcDbMText::cast(pEnt);
                    if (pText || pMText)
                    {
                        AcDbObjectId styleId;
                        if (pText)
                            styleId = pText->textStyle();
                        else
                            styleId = pMText->textStyle();

                        AcDbTextStyleTableRecord* pTsRecord = NULL;
                        if (acdbOpenObject(pTsRecord, styleId, AcDb::kForRead) == Acad::eOk)
                        {
                            AcString strName;
                            pTsRecord->getName(strName);
                            usedNames.insert(strName.kTCharPtr());
                            pTsRecord->close();
                        }
                    }
                    pEnt->close();
                }
            }
            delete pIter;
            pModelSpace->close();
        }
    }
}

int CStyleManager::CleanUnusedStyles(CString& errMsg)
{
    std::vector<TextStyleInfo> allStyles;
    if (!GetAllStyles(allStyles))
    {
        errMsg = _T("获取样式列表失败");
        return 0;
    }

    std::set<CString> usedNames;
    GetUsedStyleNames(usedNames);

    int deletedCount = 0;
    for (size_t i = 0; i < allStyles.size(); i++)
    {
        CString name = allStyles[i].name;
        if (name.CompareNoCase(_T("Standard")) == 0)
            continue;

        if (usedNames.find(name) == usedNames.end())
        {
            CString dummy;
            if (DeleteStyle(name, dummy))
                deletedCount++;
        }
    }

    return deletedCount;
}

int CStyleManager::BatchReplaceFont(const std::vector<CString>& styleNames,
                                    const CString& keyword, bool fuzzyMatch,
                                    const CString& fontFile, const CString& bigFont,
                                    CString& errMsg)
{
    std::vector<CString> targetStyles;

    // 从选择的样式列表中收集
    for (size_t i = 0; i < styleNames.size(); i++)
    {
        targetStyles.push_back(styleNames[i]);
    }

    // 如果有关键字，通过关键字匹配
    if (!keyword.IsEmpty())
    {
        std::vector<TextStyleInfo> allStyles;
        if (GetAllStyles(allStyles))
        {
            for (size_t i = 0; i < allStyles.size(); i++)
            {
                CString name = allStyles[i].name;
                bool match = false;
                if (fuzzyMatch)
                    match = (name.Find(keyword) != -1);
                else
                    match = (name.CompareNoCase(keyword) == 0);

                if (match)
                {
                    // 避免重复
                    bool found = false;
                    for (size_t j = 0; j < targetStyles.size(); j++)
                    {
                        if (targetStyles[j].CompareNoCase(name) == 0)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                        targetStyles.push_back(name);
                }
            }
        }
    }

    if (targetStyles.empty())
    {
        errMsg = _T("没有匹配的样式");
        return 0;
    }

    int updatedCount = 0;
    for (size_t i = 0; i < targetStyles.size(); i++)
    {
        CString dummy;
        if (UpdateStyleInfo(targetStyles[i], fontFile, bigFont, -1, 0, dummy))
            updatedCount++;
    }

    return updatedCount;
}

bool CStyleManager::UnifyTextStyles(const CString& targetStyle, bool useSelection,
                                    int& affectedCount, CString& errMsg)
{
    affectedCount = 0;

    AcDbDatabase* pDb = GetWorkingDatabase();
    if (!pDb)
    {
        errMsg = _T("无法获取数据库");
        return false;
    }

    // 获取目标样式ID
    AcDbTextStyleTable* pTable = NULL;
    Acad::ErrorStatus es = pDb->getTextStyleTable(pTable, AcDb::kForRead);
    if (es != Acad::eOk)
    {
        errMsg = _T("无法打开文字样式表");
        return false;
    }

    AcDbTextStyleTableRecord* pTargetRecord = NULL;
    es = pTable->getAt(targetStyle, pTargetRecord, AcDb::kForRead);
    pTable->close();
    if (es != Acad::eOk)
    {
        errMsg = _T("目标样式不存在");
        return false;
    }
    AcDbObjectId targetStyleId = pTargetRecord->objectId();
    pTargetRecord->close();

    if (useSelection)
    {
        // 通过选择集处理
        ads_name ssName;
        if (acedSSGet(NULL, NULL, NULL, NULL, ssName) != RTNORM)
        {
            errMsg = _T("未选择对象");
            return false;
        }

        long len = 0;
        acedSSLength(ssName, &len);
        for (long i = 0; i < len; i++)
        {
            ads_name entName;
            acedSSName(ssName, i, entName);

            AcDbObjectId entId;
            acdbGetObjectId(entId, entName);

            AcDbEntity* pEnt = NULL;
            if (acdbOpenObject(pEnt, entId, AcDb::kForWrite) == Acad::eOk)
            {
                AcDbText* pText = AcDbText::cast(pEnt);
                AcDbMText* pMText = AcDbMText::cast(pEnt);
                if (pText)
                {
                    pText->setTextStyle(targetStyleId);
                    affectedCount++;
                }
                else if (pMText)
                {
                    pMText->setTextStyle(targetStyleId);
                    affectedCount++;
                }
                pEnt->close();
            }
        }
        acedSSFree(ssName);
    }
    else
    {
        // 遍历模型空间所有文字对象
        AcDbBlockTable* pBlockTable = NULL;
        es = pDb->getBlockTable(pBlockTable, AcDb::kForRead);
        if (es != Acad::eOk)
        {
            errMsg = _T("无法打开块表");
            return false;
        }

        AcDbBlockTableRecord* pModelSpace = NULL;
        es = pBlockTable->getAt(ACDB_MODEL_SPACE, pModelSpace, AcDb::kForRead);
        pBlockTable->close();
        if (es != Acad::eOk)
        {
            errMsg = _T("无法打开模型空间");
            return false;
        }

        AcDbBlockTableRecordIterator* pIter = NULL;
        pModelSpace->newIterator(pIter);
        for (pIter->start(); !pIter->done(); pIter->step())
        {
            AcDbEntity* pEnt = NULL;
            if (pIter->getEntity(pEnt, AcDb::kForWrite) == Acad::eOk)
            {
                AcDbText* pText = AcDbText::cast(pEnt);
                AcDbMText* pMText = AcDbMText::cast(pEnt);
                if (pText)
                {
                    pText->setTextStyle(targetStyleId);
                    affectedCount++;
                }
                else if (pMText)
                {
                    pMText->setTextStyle(targetStyleId);
                    affectedCount++;
                }
                pEnt->close();
            }
        }
        delete pIter;
        pModelSpace->close();
    }

    return true;
}

bool CStyleManager::IsFontExists(const CString& fontName)
{
    if (fontName.IsEmpty())
        return true;

    CString lowerName = fontName;
    lowerName.MakeLower();

    // 检查Windows Fonts目录
    CString fontPaths[] = {
        _T("C:\\Windows\\Fonts\\") + fontName,
        _T("C:\\Windows\\Fonts\\") + lowerName,
    };

    for (int i = 0; i < 2; i++)
    {
        if (PathFileExists(fontPaths[i]))
            return true;
    }

    // 如果没有扩展名，尝试常见扩展名
    if (lowerName.Find(_T(".shx")) == -1 &&
        lowerName.Find(_T(".ttf")) == -1 &&
        lowerName.Find(_T(".ttc")) == -1)
    {
        CString extPaths[] = {
            _T("C:\\Windows\\Fonts\\") + fontName + _T(".shx"),
            _T("C:\\Windows\\Fonts\\") + fontName + _T(".ttf"),
            _T("C:\\Windows\\Fonts\\") + lowerName + _T(".shx"),
            _T("C:\\Windows\\Fonts\\") + lowerName + _T(".ttf"),
        };
        for (int i = 0; i < 4; i++)
        {
            if (PathFileExists(extPaths[i]))
                return true;
        }
    }

    return false;
}

int CStyleManager::ReplaceMissingFonts(const CString& defaultFont,
                                       const CString& defaultBigFont,
                                       const CString& defaultTtf,
                                       CString& errMsg)
{
    AcDbDatabase* pDb = GetWorkingDatabase();
    if (!pDb)
    {
        errMsg = _T("无法获取数据库");
        return 0;
    }

    AcDbTextStyleTable* pTable = NULL;
    Acad::ErrorStatus es = pDb->getTextStyleTable(pTable, AcDb::kForRead);
    if (es != Acad::eOk)
    {
        errMsg = _T("无法打开文字样式表");
        return 0;
    }

    int replacedCount = 0;
    AcDbTextStyleTableIterator* pIter = NULL;
    es = pTable->newIterator(pIter);
    if (es != Acad::eOk)
    {
        pTable->close();
        return 0;
    }

    for (pIter->start(); !pIter->done(); pIter->step())
    {
        AcDbTextStyleTableRecord* pRecord = NULL;
        es = pIter->getRecord(pRecord, AcDb::kForWrite);
        if (es != Acad::eOk)
            continue;

        AcString strFont;
        pRecord->fileName(strFont);
        CString fontFile = strFont.kTCharPtr();

        if (!fontFile.IsEmpty() && !IsFontExists(fontFile))
        {
            if (!defaultFont.IsEmpty())
            {
                pRecord->setFileName(defaultFont);
                replacedCount++;
            }
        }

        AcString strBigFont;
        pRecord->bigFontFileName(strBigFont);
        CString bigFont = strBigFont.kTCharPtr();

        if (!bigFont.IsEmpty() && !IsFontExists(bigFont))
        {
            if (!defaultBigFont.IsEmpty())
            {
                pRecord->setBigFontFileName(defaultBigFont);
                replacedCount++;
            }
        }

        pRecord->close();
    }

    delete pIter;
    pTable->close();
    return replacedCount;
}
