#include "ui/admin_widget.h"
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QFile>

AdminWidget::AdminWidget(QuestionBank& questionBank, PoemBank& poemBank, QWidget* parent)
    : QWidget(parent), m_questionBank(questionBank), m_poemBank(poemBank)
{
    setupUI();
    refreshQuestionList();
    refreshPoemList();
    refreshCategoryList();
}

void AdminWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    QTabWidget* tabWidget = new QTabWidget();
    
    // 题库管理标签页
    QWidget* questionTab = new QWidget();
    QVBoxLayout* questionLayout = new QVBoxLayout(questionTab);
    
    // 题目列表
    m_questionList = new QListWidget();
    questionLayout->addWidget(m_questionList);
    
    // 添加题目表单
    QGroupBox* questionForm = new QGroupBox("添加题目（支持批量添加）");
    QFormLayout* questionFormLayout = new QFormLayout(questionForm);
    
    m_questionContentEdit = new QLineEdit();
    m_questionContentEdit->setPlaceholderText("单句：床前明月光，疑是地上霜\n批量：床前明月光，疑是地上霜。举头望明月，低头思故乡");
    m_questionDifficultyEdit = new QLineEdit("1");
    m_questionCategoryEdit = new QLineEdit();
    m_questionCategoryEdit->setPlaceholderText("如：唐诗、宋词、五言、七言");
    
    questionFormLayout->addRow("诗句:", m_questionContentEdit);
    questionFormLayout->addRow("难度:", m_questionDifficultyEdit);
    questionFormLayout->addRow("分类:", m_questionCategoryEdit);
    
    // 添加说明
    QLabel* helpLabel = new QLabel("说明：用中文逗号分隔上下句，用句号/问号/感叹号分隔多句");
    helpLabel->setStyleSheet("color: gray; font-size: 11px;");
    questionFormLayout->addRow("", helpLabel);
    
    questionLayout->addWidget(questionForm);
    
    // 分类列表
    QGroupBox* categoryGroup = new QGroupBox("分类筛选（勾选的分类将用于新游戏）");
    QVBoxLayout* categoryLayout = new QVBoxLayout(categoryGroup);
    m_categoryList = new QListWidget();
    m_categoryList->setMaximumHeight(100);
    categoryLayout->addWidget(m_categoryList);
    questionLayout->addWidget(categoryGroup);
    
    // 按钮
    QHBoxLayout* questionBtnLayout = new QHBoxLayout();
    QPushButton* addQuestionBtn = new QPushButton("添加题目");
    QPushButton* removeQuestionBtn = new QPushButton("删除题目");
    QPushButton* exportBtn = new QPushButton("导出数据");
    QPushButton* importBtn = new QPushButton("导入数据");
    questionBtnLayout->addWidget(addQuestionBtn);
    questionBtnLayout->addWidget(removeQuestionBtn);
    questionBtnLayout->addWidget(exportBtn);
    questionBtnLayout->addWidget(importBtn);
    questionLayout->addLayout(questionBtnLayout);
    
    connect(addQuestionBtn, &QPushButton::clicked, this, &AdminWidget::onAddQuestion);
    connect(removeQuestionBtn, &QPushButton::clicked, this, &AdminWidget::onRemoveQuestion);
    connect(exportBtn, &QPushButton::clicked, this, &AdminWidget::onExportData);
    connect(importBtn, &QPushButton::clicked, this, &AdminWidget::onImportData);
    connect(m_questionList, &QListWidget::itemSelectionChanged, this, &AdminWidget::onQuestionSelectionChanged);
    
    tabWidget->addTab(questionTab, "题库管理");
    
    // 诗词库管理标签页
    QWidget* poemTab = new QWidget();
    QVBoxLayout* poemLayout = new QVBoxLayout(poemTab);
    
    // 诗词列表
    m_poemList = new QListWidget();
    poemLayout->addWidget(m_poemList);
    
    // 添加诗词表单
    QGroupBox* poemForm = new QGroupBox("添加诗词");
    QFormLayout* poemFormLayout = new QFormLayout(poemForm);
    
    m_poemTitleEdit = new QLineEdit();
    m_poemAuthorEdit = new QLineEdit();
    m_poemDynastyEdit = new QLineEdit();
    m_poemVersesEdit = new QTextEdit();
    m_poemVersesEdit->setPlaceholderText("每行一句，或用中文逗号分隔\n例如：\n床前明月光，疑是地上霜\n举头望明月，低头思故乡");
    m_poemVersesEdit->setMaximumHeight(150);
    m_poemPunctsEdit = new QLineEdit();
    m_poemPunctsEdit->setPlaceholderText("句间标点，如：，。（默认逗号）");
    m_poemAllowedEdit = new QLineEdit();
    m_poemAllowedEdit->setPlaceholderText("特殊联：上句索引,下句索引;上句索引,下句索引\n例如：,3;3,4");
    
    poemFormLayout->addRow("标题:", m_poemTitleEdit);
    poemFormLayout->addRow("作者:", m_poemAuthorEdit);
    poemFormLayout->addRow("朝代:", m_poemDynastyEdit);
    poemFormLayout->addRow("诗句:", m_poemVersesEdit);
    poemFormLayout->addRow("标点:", m_poemPunctsEdit);
    poemFormLayout->addRow("特殊:", m_poemAllowedEdit);
    
    poemLayout->addWidget(poemForm);
    
    // 按钮
    QHBoxLayout* poemBtnLayout = new QHBoxLayout();
    QPushButton* addPoemBtn = new QPushButton("添加诗词");
    QPushButton* removePoemBtn = new QPushButton("删除诗词");
    poemBtnLayout->addWidget(addPoemBtn);
    poemBtnLayout->addWidget(removePoemBtn);
    poemLayout->addLayout(poemBtnLayout);
    
    connect(addPoemBtn, &QPushButton::clicked, this, &AdminWidget::onAddPoem);
    connect(removePoemBtn, &QPushButton::clicked, this, &AdminWidget::onRemovePoem);
    connect(m_poemList, &QListWidget::itemSelectionChanged, this, &AdminWidget::onPoemSelectionChanged);
    connect(m_poemList, &QListWidget::itemDoubleClicked, this, &AdminWidget::onPoemDoubleClicked);
    
    tabWidget->addTab(poemTab, "诗词库管理");
    
    mainLayout->addWidget(tabWidget);
}

void AdminWidget::refreshQuestionList() {
    m_questionList->clear();
    auto questions = m_questionBank.getAllQuestions();
    for (const auto& q : questions) {
        QString display = QString("[%1] %2")
            .arg(q.id)
            .arg(q.content);
        if (!q.category.isEmpty()) {
            display += QString(" [%1]").arg(q.category);
        }
        m_questionList->addItem(display);
    }
}

void AdminWidget::refreshPoemList() {
    m_poemList->clear();
    auto poems = m_poemBank.getAllPoems();
    for (const auto& p : poems) {
        QString display = QString("[%1] %2 - %3")
            .arg(p.id)
            .arg(p.title)
            .arg(p.author);
        m_poemList->addItem(display);
    }
}

void AdminWidget::onAddQuestion() {
    QString content = m_questionContentEdit->text().trimmed();
    int difficulty = m_questionDifficultyEdit->text().toInt();
    QString category = m_questionCategoryEdit->text().trimmed();
    
    if (content.isEmpty()) {
        QMessageBox::warning(this, "错误", "诗句不能为空");
        return;
    }
    
    // 用句号、问号、感叹号分割多联
    QStringList couplets = content.split(QRegularExpression("[。？！]"), Qt::SkipEmptyParts);
    
    int successCount = 0;
    qint64 baseTime = QDateTime::currentMSecsSinceEpoch();
    
    for (int i = 0; i < couplets.size(); ++i) {
        QString trimmedCouplet = couplets[i].trimmed();
        if (trimmedCouplet.isEmpty()) continue;
        
        // 解析上下句（用中文逗号分隔）
        QStringList sentences = trimmedCouplet.split(QChar(L'，'), Qt::SkipEmptyParts);
        if (sentences.size() != 2) {
            QMessageBox::warning(this, "错误", 
                QString("格式错误：%1\n请用中文逗号分隔上下句").arg(trimmedCouplet));
            return;
        }
        
        // 去除标点符号，只保留汉字
        QString upperClean, lowerClean;
        for (const QChar& ch : sentences[0].trimmed()) {
            if (ch.script() == QChar::Script_Han) {
                upperClean += ch;
            }
        }
        for (const QChar& ch : sentences[1].trimmed()) {
            if (ch.script() == QChar::Script_Han) {
                lowerClean += ch;
            }
        }
        
        // 自动生成唯一ID（基础时间+索引）
        QString id = QString("q%1_%2").arg(baseTime).arg(i);
        
        // 创建上下联题
        Question q;
        q.id = id;
        q.upperSentence = upperClean;
        q.lowerSentence = lowerClean;
        q.content = upperClean + lowerClean;
        q.position = Position::Both;
        q.difficulty = difficulty;
        q.category = category;  // 设置分类
        
        if (m_questionBank.addQuestion(q)) {
            successCount++;
        }
    }
    
    if (successCount > 0) {
        m_questionBank.save();
        refreshQuestionList();
        refreshCategoryList();
        QMessageBox::information(this, "成功", 
            QString("成功添加%1个题目").arg(successCount));
        m_questionContentEdit->clear();
    } else {
        QMessageBox::warning(this, "错误", "添加失败，请检查格式");
    }
}

void AdminWidget::onRemoveQuestion() {
    auto currentItem = m_questionList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "错误", "请先选择要删除的题目");
        return;
    }
    
    auto reply = QMessageBox::question(this, "确认", "确定要删除这个题目吗？");
    if (reply == QMessageBox::Yes) {
        QString text = currentItem->text();
        QString id = text.mid(1, text.indexOf(']') - 1);
        
        if (m_questionBank.removeQuestion(id)) {
            m_questionBank.save();
            refreshQuestionList();
            refreshCategoryList();
            QMessageBox::information(this, "成功", "题目删除成功");
        }
    }
}

void AdminWidget::onAddPoem() {
    QString title = m_poemTitleEdit->text().trimmed();
    QString author = m_poemAuthorEdit->text().trimmed();
    QString dynasty = m_poemDynastyEdit->text().trimmed();
    QString versesStr = m_poemVersesEdit->toPlainText().trimmed();
    QString punctsStr = m_poemPunctsEdit->text().trimmed();
    QString allowedStr = m_poemAllowedEdit->text().trimmed();
    
    if (title.isEmpty() || versesStr.isEmpty()) {
        QMessageBox::warning(this, "错误", "标题和诗句不能为空");
        return;
    }
    
    // 自动生成ID
    QString id = QString("p%1").arg(QDateTime::currentMSecsSinceEpoch());
    
    // 解析诗句 - 每行是一联诗（两句），用中文逗号分隔上下句
    QVector<QString> verses;
    
    // 按换行分割，每行是一句
    QStringList lines = versesStr.split('\n', Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) continue;
        
        // 每行用中文逗号分割为上下两句
        QStringList parts = trimmedLine.split(QChar(L'，'), Qt::SkipEmptyParts);
        
        // 如果没有中文逗号，尝试用英文逗号
        if (parts.size() == 1) {
            parts = trimmedLine.split(',', Qt::SkipEmptyParts);
        }
        
        for (const QString& part : parts) {
            QString trimmed = part.trimmed();
            // 去除句末标点
            while (!trimmed.isEmpty() && 
                   (trimmed.endsWith(QChar(L'。')) || 
                    trimmed.endsWith(QChar(L'，')) || 
                    trimmed.endsWith(QChar(L'、')) ||
                    trimmed.endsWith(QChar(L'？')) ||
                    trimmed.endsWith(QChar(L'.')) ||
                    trimmed.endsWith(QChar(L'?')) ||
                    trimmed.endsWith(QChar(L'!')))) {
                trimmed.chop(1);
            }
            if (!trimmed.isEmpty()) {
                verses.append(trimmed);
            }
        }
    }
    
    if (verses.isEmpty()) {
        QMessageBox::warning(this, "错误", "未识别到有效诗句");
        return;
    }
    
    // 解析标点 - 按顺序对应每句
    QVector<QChar> punctuations;
    if (!punctsStr.isEmpty()) {
        for (const QChar& ch : punctsStr) {
            if (ch == L'，' || ch == L'。' || ch == L'、' || ch == L'？' || ch == L'！' ||
                ch == L'.' || ch == L',') {
                punctuations.append(ch);
            }
        }
    } else {
        // 默认：每联的上句用逗号，下句用句号（表示一联结束）
        for (int i = 0; i < verses.size(); ++i) {
            if ((i + 1) % 2 == 0) {  // 每联的下句
                punctuations.append(QChar(L'。'));  // 用句号表示一联结束
            } else {
                punctuations.append(QChar(L'，'));  // 上句用逗号
            }
        }
    }
    
    // 解析特殊情况标记 - 格式：上句索引,下句索引;上句索引,下句索引
    QSet<QPair<int, int>> specialCouplets;
    if (!allowedStr.isEmpty()) {
        QStringList coupletList = allowedStr.split(';', Qt::SkipEmptyParts);
        for (const QString& coupletStr : coupletList) {
            QStringList indices = coupletStr.trimmed().split(',', Qt::SkipEmptyParts);
            if (indices.size() == 2) {
                int upperIdx = indices[0].trimmed().toInt();
                int lowerIdx = indices[1].trimmed().toInt();
                specialCouplets.insert(qMakePair(upperIdx, lowerIdx));
            }
        }
    }
    
    Poem p;
    p.id = id;
    p.title = title;
    p.author = author;
    p.dynasty = dynasty;
    p.verses = verses;
    p.punctuations = punctuations;
    p.specialCouplets = specialCouplets;
    
    if (m_poemBank.addPoem(p)) {
        m_poemBank.save();
        refreshPoemList();
        QMessageBox::information(this, "成功", 
            QString("诗词添加成功，共%1句（%2联）").arg(verses.size()).arg(verses.size() / 2));
        
        // 清空输入
        m_poemTitleEdit->clear();
        m_poemAuthorEdit->clear();
        m_poemDynastyEdit->clear();
        m_poemVersesEdit->clear();
        m_poemPunctsEdit->clear();
        m_poemAllowedEdit->clear();
    } else {
        QMessageBox::warning(this, "错误", "ID已存在");
    }
}

void AdminWidget::onRemovePoem() {
    auto currentItem = m_poemList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "错误", "请先选择要删除的诗词");
        return;
    }
    
    auto reply = QMessageBox::question(this, "确认", "确定要删除这首诗词吗？");
    if (reply == QMessageBox::Yes) {
        QString text = currentItem->text();
        QString id = text.mid(1, text.indexOf(']') - 1);
        
        if (m_poemBank.removePoem(id)) {
            m_poemBank.save();
            refreshPoemList();
            QMessageBox::information(this, "成功", "诗词删除成功");
        }
    }
}

void AdminWidget::onQuestionSelectionChanged() {
    // 可以在这里实现选中题目后的详细信息显示
}

void AdminWidget::onPoemSelectionChanged() {
    // 可以在这里实现选中诗词后的详细信息显示
}

void AdminWidget::onPoemDoubleClicked(QListWidgetItem* item) {
    if (!item) {
        return;
    }
    
    // 获取诗词ID
    QString text = item->text();
    QString id = text.mid(1, text.indexOf(']') - 1);
    
    // 查找诗词
    auto poem = m_poemBank.getPoemById(id);
    if (!poem.has_value()) {
        return;
    }
    
    // 构建详细信息
    QString info;
    info += QString("标题：%1\n").arg(poem->title);
    info += QString("作者：%1\n").arg(poem->author);
    info += QString("朝代：%1\n").arg(poem->dynasty);
    info += "\n诗句：\n";
    
    for (int i = 0; i < poem->verses.size(); ++i) {
        info += QString("%1. %2").arg(i + 1).arg(poem->verses[i]);
        if (i < poem->punctuations.size()) {
            info += poem->punctuations[i];
        }
        info += "\n";
    }
    
    // 显示详细信息
    QMessageBox::information(this, poem->title, info);
}

void AdminWidget::onExportData() {
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getSaveFileName(this, "导出数据",
        defaultPath + "/guess_poetry_data", "JSON Files (*.json)");

    if (path.isEmpty()) {
        return;
    }

    // 先保存当前数据
    m_questionBank.save();
    m_poemBank.save();

    // 从项目根目录的resources复制数据文件
    QString appPath = QCoreApplication::applicationDirPath();
    QDir dir(appPath);
    dir.cdUp();  // 从bin回到项目根目录
    QString questionsFile = dir.absolutePath() + "/resources/questions.json";
    QString poemsFile = dir.absolutePath() + "/resources/poems.json";

    // 移除目标文件（如果存在）
    QString destQuestions = path + ".questions.json";
    QString destPoems = path + ".poems.json";
    QFile::remove(destQuestions);
    QFile::remove(destPoems);

    if (QFile::copy(questionsFile, destQuestions) &&
        QFile::copy(poemsFile, destPoems)) {
        QMessageBox::information(this, "成功", "数据导出成功！\n位置：" + path);
    } else {
        QMessageBox::warning(this, "错误", "数据导出失败");
    }
}

void AdminWidget::onImportData() {
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString path = QFileDialog::getOpenFileName(this, "导入数据",
        defaultPath, "JSON Files (*.json)");

    if (path.isEmpty()) {
        return;
    }

    // 根据选择的文件推导questions和poems文件路径
    QString questionsFile = path;
    if (!questionsFile.endsWith(".questions.json")) {
        questionsFile = path.left(path.lastIndexOf('.')) + ".questions.json";
    }
    QString poemsFile = questionsFile.left(questionsFile.lastIndexOf('.')) + ".poems.json";

    if (!QFile::exists(questionsFile) || !QFile::exists(poemsFile)) {
        QMessageBox::warning(this, "错误", "找不到配套的数据文件");
        return;
    }

    // 询问是否替换
    auto reply = QMessageBox::question(this, "确认导入",
        "导入将替换当前数据！\n选择\"Yes\"替换，选择\"No\"取消");

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 复制文件到项目根目录的resources
    QString appPath = QCoreApplication::applicationDirPath();
    QDir dir(appPath);
    dir.cdUp();  // 从bin回到项目根目录
    QString destQuestions = dir.absolutePath() + "/resources/questions.json";
    QString destPoems = dir.absolutePath() + "/resources/poems.json";

    QFile::remove(destQuestions);
    QFile::remove(destPoems);

    if (QFile::copy(questionsFile, destQuestions) &&
        QFile::copy(poemsFile, destPoems)) {
        // 重新加载数据
        m_questionBank.load();
        m_poemBank.load();
        refreshQuestionList();
        refreshPoemList();
        refreshCategoryList();
        QMessageBox::information(this, "成功", "数据导入成功！");
    } else {
        QMessageBox::warning(this, "错误", "数据导入失败");
    }
}

void AdminWidget::refreshCategoryList() {
    m_categoryList->clear();
    m_categoryCheckBoxes.clear();
    
    QSet<QString> categories = m_questionBank.getAllCategories();
    
    for (const QString& category : categories) {
        QListWidgetItem* item = new QListWidgetItem(category);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);  // 默认勾选
        m_categoryList->addItem(item);
    }
    
    // 添加"全选"和"全不选"按钮
    if (!categories.isEmpty()) {
        QListWidgetItem* selectAllItem = new QListWidgetItem("[全选]");
        selectAllItem->setForeground(QColor(Qt::blue));
        m_categoryList->insertItem(0, selectAllItem);
        
        QListWidgetItem* deselectAllItem = new QListWidgetItem("[全不选]");
        deselectAllItem->setForeground(QColor(Qt::blue));
        m_categoryList->insertItem(1, deselectAllItem);
    }
}
