#include "systemnodelist.h"
#include "ui_systemnodelist.h"

#include "sync.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "activesystemnode.h"
//#include "systemnode-budget.h"
#include "systemnode-sync.h"
#include "systemnodeconfig.h"
#include "systemnodeman.h"
#include "wallet.h"
#include "init.h"
#include "guiutil.h"

#include <QTimer>
#include <QMessageBox>

CCriticalSection cs_systemnodes;

SystemnodeList::SystemnodeList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SystemnodeList),
    clientModel(0),
    walletModel(0)
{
    ui->setupUi(this);

    ui->startButton->setEnabled(false);
    ui->voteManyYesButton->setEnabled(false);
    ui->voteManyNoButton->setEnabled(false);

    ui->tableWidgetMySystemnodes->setContextMenuPolicy(Qt::CustomContextMenu);

    QAction *startAliasAction = new QAction(tr("Start alias"), this);
    contextMenu = new QMenu();
    contextMenu->addAction(startAliasAction);
    connect(ui->tableWidgetMySystemnodes, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    connect(startAliasAction, SIGNAL(triggered()), this, SLOT(on_startButton_clicked()));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateNodeList()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateMyNodeList()));
    timer->start(1000);

    updateNodeList();

    //CBlockIndex* pindexPrev = chainActive.Tip();
    //int nNext = pindexPrev->nHeight - pindexPrev->nHeight % GetBudgetPaymentCycleBlocks() + GetBudgetPaymentCycleBlocks();
    //ui->superblockLabel->setText(QString::number(nNext));
}

SystemnodeList::~SystemnodeList()
{
    delete ui;
}

void SystemnodeList::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
        // try to update list when systemnode count changes
        connect(clientModel, SIGNAL(strSystemnodesChanged(QString)), this, SLOT(updateNodeList()));
    }
}

void SystemnodeList::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
}

void SystemnodeList::showContextMenu(const QPoint &point)
{
    QTableWidgetItem *item = ui->tableWidgetMySystemnodes->itemAt(point);
    if(item) contextMenu->exec(QCursor::pos());
}

void SystemnodeList::StartAlias(std::string strAlias)
{
    std::string statusObj;
    statusObj += "<center>Alias: " + strAlias;

    BOOST_FOREACH(CSystemnodeConfig::CSystemnodeEntry mne, systemnodeConfig.getEntries()) {
        if(mne.getAlias() == strAlias) {
            std::string errorMessage;
            CSystemnodeBroadcast mnb;

            bool result = CSystemnodeBroadcast::Create(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb);

            if(result) {
                statusObj += "<br>Successfully started systemnode." ;
                snodeman.UpdateSystemnodeList(mnb);
                mnb.Relay();
            } else {
                statusObj += "<br>Failed to start systemnode.<br>Error: " + errorMessage;
            }
            break;
        }
    }
    statusObj += "</center>";

    QMessageBox msg;
    msg.setText(QString::fromStdString(statusObj));

    msg.exec();
}

void SystemnodeList::StartAll(std::string strCommand)
{
    int total = 0;
    int successful = 0;
    int fail = 0;
    std::string statusObj;

    BOOST_FOREACH(CSystemnodeConfig::CSystemnodeEntry mne, systemnodeConfig.getEntries()) {
        std::string errorMessage;
        CSystemnodeBroadcast mnb;

        CTxIn vin = CTxIn(uint256S(mne.getTxHash()), uint32_t(atoi(mne.getOutputIndex().c_str())));
        CSystemnode *pmn = snodeman.Find(vin);

        if(strCommand == "start-missing" && pmn) continue;

        bool result = CSystemnodeBroadcast::Create(mne.getIp(), mne.getPrivKey(), mne.getTxHash(), mne.getOutputIndex(), errorMessage, mnb);

        if(result) {
            successful++;
            snodeman.UpdateSystemnodeList(mnb);
            mnb.Relay();
        } else {
            fail++;
            statusObj += "\nFailed to start " + mne.getAlias() + ". Error: " + errorMessage;
        }
        total++;
    }
    pwalletMain->Lock();

    std::string returnObj;
    returnObj = strprintf("Successfully started %d systemnodes, failed to start %d, total %d", successful, fail, total);
    if (fail > 0)
        returnObj += statusObj;

    QMessageBox msg;
    msg.setText(QString::fromStdString(returnObj));
    msg.exec();
}

void SystemnodeList::updateMySystemnodeInfo(QString alias, QString addr, QString privkey, QString txHash, QString txIndex, CSystemnode *pmn)
{
    LOCK(cs_mnlistupdate);
    bool bFound = false;
    int nodeRow = 0;

    for(int i=0; i < ui->tableWidgetMySystemnodes->rowCount(); i++)
    {
        if(ui->tableWidgetMySystemnodes->item(i, 0)->text() == alias)
        {
            bFound = true;
            nodeRow = i;
            break;
        }
    }

    if(nodeRow == 0 && !bFound) {
        nodeRow = ui->tableWidgetMySystemnodes->rowCount();
        ui->tableWidgetMySystemnodes->insertRow(nodeRow);
    }

    QTableWidgetItem *aliasItem = new QTableWidgetItem(alias);
    QTableWidgetItem *addrItem = new QTableWidgetItem(addr);
    QTableWidgetItem *protocolItem = new QTableWidgetItem(QString::number(pmn ? pmn->protocolVersion : -1));
    QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(pmn ? pmn->Status() : "MISSING"));
    QTableWidgetItem *activeSecondsItem = new QTableWidgetItem(QString::fromStdString(DurationToDHMS(pmn ? (pmn->lastPing.sigTime - pmn->sigTime) : 0)));
    QTableWidgetItem *lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M", pmn ? pmn->lastPing.sigTime : 0)));
    QTableWidgetItem *pubkeyItem = new QTableWidgetItem(QString::fromStdString(pmn ? CBitcoinAddress(pmn->pubkey.GetID()).ToString() : ""));

    ui->tableWidgetMySystemnodes->setItem(nodeRow, 0, aliasItem);
    ui->tableWidgetMySystemnodes->setItem(nodeRow, 1, addrItem);
    ui->tableWidgetMySystemnodes->setItem(nodeRow, 2, protocolItem);
    ui->tableWidgetMySystemnodes->setItem(nodeRow, 3, statusItem);
    ui->tableWidgetMySystemnodes->setItem(nodeRow, 4, activeSecondsItem);
    ui->tableWidgetMySystemnodes->setItem(nodeRow, 5, lastSeenItem);
    ui->tableWidgetMySystemnodes->setItem(nodeRow, 6, pubkeyItem);
}

void SystemnodeList::updateMyNodeList(bool reset) {
    static int64_t lastMyListUpdate = 0;

    // automatically update my systemnode list only once in MY_SYSTEMNODELIST_UPDATE_SECONDS seconds,
    // this update still can be triggered manually at any time via button click
    int64_t timeTillUpdate = lastMyListUpdate + MY_SYSTEMNODELIST_UPDATE_SECONDS - GetTime();
    ui->secondsLabel->setText(QString::number(timeTillUpdate));

    if(timeTillUpdate > 0 && !reset) return;
    lastMyListUpdate = GetTime();

    ui->tableWidgetSystemnodes->setSortingEnabled(false);
    BOOST_FOREACH(CSystemnodeConfig::CSystemnodeEntry mne, systemnodeConfig.getEntries()) {
        CTxIn vin = CTxIn(uint256S(mne.getTxHash()), uint32_t(atoi(mne.getOutputIndex().c_str())));
        CSystemnode *pmn = snodeman.Find(vin);

        updateMySystemnodeInfo(QString::fromStdString(mne.getAlias()), QString::fromStdString(mne.getIp()), QString::fromStdString(mne.getPrivKey()), QString::fromStdString(mne.getTxHash()),
            QString::fromStdString(mne.getOutputIndex()), pmn);
    }
    ui->tableWidgetSystemnodes->setSortingEnabled(true);

    // reset "timer"
    ui->secondsLabel->setText("0");
}

void SystemnodeList::updateNodeList()
{
    static int64_t nTimeListUpdate = 0;

      // to prevent high cpu usage update only once in SYSTEMNODELIST_UPDATE_SECONDS secondsLabel
      // or SYSTEMNODELIST_FILTER_COOLDOWN_SECONDS seconds after filter was last changed
      int64_t nTimeToWait =   fFilterUpdated
                              ? nTimeFilterUpdate - GetTime() + SYSTEMNODELIST_FILTER_COOLDOWN_SECONDS
                              : nTimeListUpdate - GetTime() + SYSTEMNODELIST_UPDATE_SECONDS;

    if(fFilterUpdated) ui->countLabel->setText(QString::fromStdString(strprintf("Please wait... %d", nTimeToWait)));
    if(nTimeToWait > 0) return;

    nTimeListUpdate = GetTime();
    fFilterUpdated = false;

    TRY_LOCK(cs_systemnodes, lockSystemnodes);
    if(!lockSystemnodes)
        return;

    QString strToFilter;
    ui->countLabel->setText("Updating...");
    ui->tableWidgetSystemnodes->setSortingEnabled(false);
    ui->tableWidgetSystemnodes->clearContents();
    ui->tableWidgetSystemnodes->setRowCount(0);
    std::vector<CSystemnode> vSystemnodes = snodeman.GetFullSystemnodeVector();

    BOOST_FOREACH(CSystemnode& mn, vSystemnodes)
    {
        // populate list
        // Address, Protocol, Status, Active Seconds, Last Seen, Pub Key
        QTableWidgetItem *addressItem = new QTableWidgetItem(QString::fromStdString(mn.addr.ToString()));
        QTableWidgetItem *protocolItem = new QTableWidgetItem(QString::number(mn.protocolVersion));
        QTableWidgetItem *statusItem = new QTableWidgetItem(QString::fromStdString(mn.Status()));
        QTableWidgetItem *activeSecondsItem = new QTableWidgetItem(QString::fromStdString(DurationToDHMS(mn.lastPing.sigTime - mn.sigTime)));
        QTableWidgetItem *lastSeenItem = new QTableWidgetItem(QString::fromStdString(DateTimeStrFormat("%Y-%m-%d %H:%M", mn.lastPing.sigTime)));
        QTableWidgetItem *pubkeyItem = new QTableWidgetItem(QString::fromStdString(CBitcoinAddress(mn.pubkey.GetID()).ToString()));

        if (strCurrentFilter != "")
        {
            strToFilter =   addressItem->text() + " " +
                            protocolItem->text() + " " +
                            statusItem->text() + " " +
                            activeSecondsItem->text() + " " +
                            lastSeenItem->text() + " " +
                            pubkeyItem->text();
            if (!strToFilter.contains(strCurrentFilter)) continue;
        }

        ui->tableWidgetSystemnodes->insertRow(0);
        ui->tableWidgetSystemnodes->setItem(0, 0, addressItem);
        ui->tableWidgetSystemnodes->setItem(0, 1, protocolItem);
        ui->tableWidgetSystemnodes->setItem(0, 2, statusItem);
        ui->tableWidgetSystemnodes->setItem(0, 3, activeSecondsItem);
        ui->tableWidgetSystemnodes->setItem(0, 4, lastSeenItem);
        ui->tableWidgetSystemnodes->setItem(0, 5, pubkeyItem);
    }

    ui->countLabel->setText(QString::number(ui->tableWidgetSystemnodes->rowCount()));
    ui->tableWidgetSystemnodes->setSortingEnabled(true);

}

void SystemnodeList::on_filterLineEdit_textChanged(const QString &filterString) {
    strCurrentFilter = filterString;
    nTimeFilterUpdate = GetTime();
    fFilterUpdated = true;
    ui->countLabel->setText(QString::fromStdString(strprintf("Please wait... %d", SYSTEMNODELIST_FILTER_COOLDOWN_SECONDS)));
}

void SystemnodeList::on_startButton_clicked()
{
    // Find selected node alias
    QItemSelectionModel* selectionModel = ui->tableWidgetMySystemnodes->selectionModel();
    QModelIndexList selected = selectionModel->selectedRows();
    if(selected.count() == 0)
        return;

    QModelIndex index = selected.at(0);
    int r = index.row();
    std::string strAlias = ui->tableWidgetMySystemnodes->item(r, 0)->text().toStdString();

    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm systemnode start"),
        tr("Are you sure you want to start systemnode %1?").arg(QString::fromStdString(strAlias)),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        return;
    }

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if(encStatus == walletModel->Locked)
    {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock(true));
        if(!ctx.isValid())
        {
            // Unlock wallet was cancelled
            return;
        }
        StartAlias(strAlias);
        return;
    }

    StartAlias(strAlias);
}

void SystemnodeList::on_startAllButton_clicked()
{
    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this, tr("Confirm all systemnodes start"),
        tr("Are you sure you want to start ALL systemnodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        return;
    }

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if(encStatus == walletModel->Locked)
    {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock(true));
        if(!ctx.isValid())
        {
            // Unlock wallet was cancelled
            return;
        }
        StartAll();
        return;
    }

    StartAll();
}

void SystemnodeList::on_startMissingButton_clicked()
{

    if(systemnodeSync.RequestedSystemnodeAssets <= SYSTEMNODE_SYNC_LIST ||
      systemnodeSync.RequestedSystemnodeAssets == SYSTEMNODE_SYNC_FAILED) {
        QMessageBox::critical(this, tr("Command is not available right now"),
            tr("You can't use this command until systemnode list is synced"));
        return;
    }

    // Display message box
    QMessageBox::StandardButton retval = QMessageBox::question(this,
        tr("Confirm missing systemnodes start"),
        tr("Are you sure you want to start MISSING systemnodes?"),
        QMessageBox::Yes | QMessageBox::Cancel,
        QMessageBox::Cancel);

    if(retval != QMessageBox::Yes)
    {
        return;
    }

    WalletModel::EncryptionStatus encStatus = walletModel->getEncryptionStatus();
    if(encStatus == walletModel->Locked)
    {
        WalletModel::UnlockContext ctx(walletModel->requestUnlock(true));
        if(!ctx.isValid())
        {
            // Unlock wallet was cancelled
            return;
        }
        StartAll("start-missing");
        return;
    }

    StartAll("start-missing");
}

void SystemnodeList::on_tableWidgetMySystemnodes_itemSelectionChanged()
{
    if(ui->tableWidgetMySystemnodes->selectedItems().count() > 0)
    {
        ui->startButton->setEnabled(true);
    }
}

void SystemnodeList::on_UpdateButton_clicked()
{
    updateMyNodeList(true);
}
