#include "ui/main_window.h"
#include <QMessageBox>
#include <QDir>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // 初始化数据存储 - 使用项目根目录的resources
    QString appPath = QCoreApplication::applicationDirPath();
    // 从bin目录向上找到项目根目录
    QDir dir(appPath);
    dir.cdUp();  // 从bin回到项目根目录
    QString dataPath = dir.absolutePath() + "/resources";
    QDir().mkpath(dataPath);  // 确保资源目录存在

    m_storage = new FileStorage(dataPath);
    m_questionBank = new QuestionBank(*m_storage);
    m_poemBank = new PoemBank(*m_storage);
    m_gameController = new GameController(*m_questionBank, *m_poemBank);
    
    // 加载数据
    loadData();
    
    // 设置菜单栏
    setupMenuBar();
    
    // 创建堆叠窗口
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);
    
    // 创建游戏界面和管理界面
    m_gameWidget = new GameWidget(*m_gameController, this);
    m_adminWidget = new AdminWidget(*m_questionBank, *m_poemBank, this);
    
    m_stackedWidget->addWidget(m_gameWidget);
    m_stackedWidget->addWidget(m_adminWidget);
    
    // 默认显示游戏界面
    showGame();
    
    // 自动开始新游戏
    startNewGame();
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = this->menuBar();
    
    // 游戏菜单
    QMenu* gameMenu = menuBar->addMenu("游戏");
    QAction* newGameAction = gameMenu->addAction("新游戏");
    connect(newGameAction, &QAction::triggered, this, &MainWindow::startNewGame);
    
    // 切换菜单
    QMenu* viewMenu = menuBar->addMenu("视图");
    QAction* gameViewAction = viewMenu->addAction("游戏界面");
    QAction* adminViewAction = viewMenu->addAction("管理界面");
    connect(gameViewAction, &QAction::triggered, this, &MainWindow::showGame);
    connect(adminViewAction, &QAction::triggered, this, &MainWindow::showAdmin);
}

void MainWindow::loadData() {
    bool questionLoaded = m_questionBank->load();
    bool poemLoaded = m_poemBank->load();
    
    // 如果数据加载失败，创建示例数据
    if (!questionLoaded || m_questionBank->isEmpty()) {
        // 创建示例题库 - 全部使用上下联格式
        Question q1;
        q1.id = "q001";
        q1.upperSentence = "床前明月光";
        q1.lowerSentence = "疑是地上霜";
        q1.content = "床前明月光疑是地上霜";
        q1.position = Position::Both;
        q1.poem_id = "p001";
        q1.difficulty = 1;
        m_questionBank->addQuestion(q1);
        
        Question q2;
        q2.id = "q002";
        q2.upperSentence = "举头望明月";
        q2.lowerSentence = "低头思故乡";
        q2.content = "举头望明月低头思故乡";
        q2.position = Position::Both;
        q2.poem_id = "p001";
        q2.difficulty = 1;
        m_questionBank->addQuestion(q2);
        
        Question q3;
        q3.id = "q003";
        q3.upperSentence = "白日依山尽";
        q3.lowerSentence = "黄河入海流";
        q3.content = "白日依山尽黄河入海流";
        q3.position = Position::Both;
        q3.poem_id = "p002";
        q3.difficulty = 1;
        m_questionBank->addQuestion(q3);
        
        Question q4;
        q4.id = "q004";
        q4.upperSentence = "欲穷千里目";
        q4.lowerSentence = "更上一层楼";
        q4.content = "欲穷千里目更上一层楼";
        q4.position = Position::Both;
        q4.poem_id = "p002";
        q4.difficulty = 1;
        m_questionBank->addQuestion(q4);
        
        m_questionBank->save();
    }
    
    if (!poemLoaded || m_poemBank->isEmpty()) {
        // 创建示例诗词库
        Poem p1;
        p1.id = "p001";
        p1.title = "静夜思";
        p1.author = "李白";
        p1.dynasty = "唐";
        p1.verses = {"床前明月光", "疑是地上霜", "举头望明月", "低头思故乡"};
        p1.punctuations = {QChar(L'，'), QChar(L'，'), QChar(L'，'), QChar(L'。')};
        m_poemBank->addPoem(p1);
        
        Poem p2;
        p2.id = "p002";
        p2.title = "登鹳雀楼";
        p2.author = "王之涣";
        p2.dynasty = "唐";
        p2.verses = {"白日依山尽", "黄河入海流", "欲穷千里目", "更上一层楼"};
        p2.punctuations = {QChar(L'，'), QChar(L'。'), QChar(L'，'), QChar(L'。')};
        // 标准联：(0,1), (2,3)
        m_poemBank->addPoem(p2);
        
        // 添加更多诗词
        Poem p3;
        p3.id = "p003";
        p3.title = "相思";
        p3.author = "王维";
        p3.dynasty = "唐";
        p3.verses = {"红豆生南国", "春来发几枝", "愿君多采撷", "此物最相思"};
        p3.punctuations = {QChar(L'，'), QChar(L'。'), QChar(L'，'), QChar(L'。')};
        m_poemBank->addPoem(p3);
        
        m_poemBank->save();
    }
}

void MainWindow::showGame() {
    m_stackedWidget->setCurrentWidget(m_gameWidget);
}

void MainWindow::showAdmin() {
    m_stackedWidget->setCurrentWidget(m_adminWidget);
}

void MainWindow::startNewGame() {
    auto state = m_gameController->startNewGame();
    if (!state.has_value()) {
        QMessageBox::warning(this, "错误", "题库为空，请先添加题目！");
        showAdmin();
        return;
    }
    
    m_gameWidget->reset();
    m_gameWidget->displayQuestion(state->currentQuestion);
}
