#ifndef FILEEXPLORE_H
#define FILEEXPLORE_H

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileSystemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QStack>
#include <QTreeView>
#include <QVBoxLayout>

class FileExplorer : public QMainWindow
{
    Q_OBJECT

  public:
    FileExplorer(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        // 创建文件系统模型
        fileSystemModel = new QFileSystemModel(this);
        fileSystemModel->setRootPath(QDir::rootPath());

        // 创建 QTreeView 用于显示文件夹和文件
        treeView = new QTreeView(this);
        treeView->setModel(fileSystemModel);
        treeView->setColumnWidth(0, 250); // 设置列宽
        treeView->setRootIndex(
            fileSystemModel->index(QDir::rootPath())); // 设置根目录

        // 创建路径显示标签
        pathLabel = new QLabel("当前路径: ", this);
        pathLabel->setAlignment(Qt::AlignLeft);

        // 创建上一步和下一步按钮
        QPushButton *backButton = new QPushButton("上一步", this);
        QPushButton *forwardButton = new QPushButton("下一步", this);

        // 创建布局
        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->addWidget(pathLabel);

        // 布局按钮
        QHBoxLayout *buttonLayout = new QHBoxLayout;
        buttonLayout->addWidget(backButton);
        buttonLayout->addWidget(forwardButton);

        mainLayout->addLayout(buttonLayout);
        mainLayout->addWidget(treeView);

        // 创建中心部件并设置布局
        QWidget *centralWidget = new QWidget(this);
        centralWidget->setLayout(mainLayout);
        setCentralWidget(centralWidget);

        // 当前路径的历史记录
        historyStack.push(QDir::rootPath()); // 初始根路径
        currentPathIndex = 0;

        // 设置窗口标题
        setWindowTitle("文件资源管理器");

        // 连接信号和槽
        connect(
            treeView,
            &QTreeView::doubleClicked,
            this,
            &FileExplorer::onItemDoubleClicked);
        connect(
            backButton,
            &QPushButton::clicked,
            this,
            &FileExplorer::onBackClicked);
        connect(
            forwardButton,
            &QPushButton::clicked,
            this,
            &FileExplorer::onForwardClicked);

        // 初始路径显示
        updateCurrentPathDisplay();
        resize(800, 600);
    }

  private slots:
    // 双击文件夹时进入子目录
    void onItemDoubleClicked(const QModelIndex &index)
    {
        QString path = fileSystemModel->filePath(index);

        if (QFileInfo(path).isDir()) {
            historyStack.push(path); // 记录历史路径
            currentPathIndex++;
            updateCurrentPathDisplay();
            treeView->setRootIndex(
                fileSystemModel->index(path)); // 进入子文件夹
        } else {
            qDebug() << "双击了文件: " << path;
        }
    }

    // 上一步按钮
    void onBackClicked()
    {
        if (currentPathIndex > 0) {
            currentPathIndex--;
            QString previousPath = historyStack[currentPathIndex];
            treeView->setRootIndex(
                fileSystemModel->index(previousPath)); // 返回上级目录
            updateCurrentPathDisplay();
        }
    }

    // 下一步按钮
    void onForwardClicked()
    {
        if (currentPathIndex < historyStack.size() - 1) {
            currentPathIndex++;
            QString nextPath = historyStack[currentPathIndex];
            treeView->setRootIndex(
                fileSystemModel->index(nextPath)); // 进入下级目录
            updateCurrentPathDisplay();
        }
    }

  private:
    // 更新当前路径显示
    void updateCurrentPathDisplay()
    {
        QString currentPath = historyStack[currentPathIndex];
        pathLabel->setText("当前路径: " + currentPath);
    }

  private:
    QTreeView *treeView;               // 用于显示文件树的视图
    QFileSystemModel *fileSystemModel; // 用于处理文件系统的模型
    QLabel *pathLabel;                 // 用于显示当前路径的标签
    QStack<QString> historyStack;      // 路径历史记录栈
    int currentPathIndex;              // 当前路径索引
};

#endif // FILEEXPLORE_H
