﻿#include "game.h"
#include "gameitems/gameboard.h"
#include <QColor>
#include <QDebug>
#include <math.h>
#include "button.h"
#include "gameitems/queen.h"

int Piece::deadBlack = 0;
int Piece::deadWhite = 0;

game::game(QWidget *parent):QGraphicsView(parent)
{
    //Making the Scene
    board = NULL;
    gameScene = new QGraphicsScene();
    gameScene->setSceneRect(0,0,1400,840);
    piece_to_placed = NULL; // important, do not forgot to initiate it. 

    //Making the view
    setFixedSize(1400,850);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(gameScene);
    SetGamecolor();

}


void game::placeTheBoard()
{
    board = new gameboard(playerside);
    board->placeBoxes();
}


void game::addToScene(QGraphicsItem *item)
{
    gameScene->addItem(item);
}

void game::start() // playagain use this
{
    delete Siri;
    Siri =NULL;
    AIsSide = -1;
    playerside = 0;
    onlineGame = false;
    gameScene->clear();
    playOffline();
    addToScene(turnDisplay);
    addToScene(check);
    placeTheBoard();
    placePieces();
}

void game::register_user() {
    QDialog dialog;
    dialog.setWindowTitle("Sign Up an Account");
    // Create two line edit widgets
    QLineEdit text1LineEdit;
    QLineEdit text2LineEdit;
    QLineEdit text3LineEdit;
    QLineEdit text4LineEdit;
    QLineEdit text5LineEdit;
    text1LineEdit.setText("127.0.0.1");
    // Create a layout for the dialog
    QFormLayout layout(&dialog);
    layout.addRow("Server IP address:", &text1LineEdit);
    layout.addRow("ID:", &text2LineEdit);
    layout.addRow("Password:", &text3LineEdit);
    layout.addRow("Confirm password:", &text4LineEdit);
    layout.addRow("Verification code:", &text5LineEdit);
    // Create an OK button
    QPushButton okButton("Register");
    layout.addRow(&okButton);
    // Connect the OK button's clicked signal to close the dialog
    QObject::connect(&okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    // Show the dialog and wait for user input
    if (dialog.exec() == QDialog::Accepted) {
        QString in_txt[5] = {text1LineEdit.text(), text2LineEdit.text(), text3LineEdit.text(),
                                    text4LineEdit.text(), text5LineEdit.text()};
        for(int i = 0; i < 5; i++)
            if(in_txt[i].isEmpty()) {
                QMessageBox::critical(NULL, "Error", "Missing Input\nPlease try again!");
                return;
            }
        if(in_txt[4] != "123321") {
            QMessageBox::critical(NULL, "Error", "Wrong verification code to register!");
            return;
        }
        if(in_txt[2] != in_txt[3]) {
            QMessageBox::critical(NULL, "Error", "The passwords are different!");
            return;
        }
        if(in_txt[1].contains(" ") || in_txt[2].contains(" ") || in_txt[1].length() < 5 || in_txt[2].length() < 8) {
            QMessageBox::critical(NULL, "Error", "Your ID or Password does not match syntax!\nMinimum length of ID(5), password(8)\nID and password cannot contains space character");
            return;
        }
        int socfd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in sAdds;
        sAdds.sin_family = AF_INET;
        sAdds.sin_port = htons(1111);
        sAdds.sin_addr.s_addr = inet_addr(in_txt[0].toUtf8().constData());
        if (::connect(socfd, (struct sockaddr *)&sAdds, sizeof(sAdds))) {
            QMessageBox::critical(NULL, "Error", "Failed to Connect");
            return;
        }
        cJSON * Mesg;
        Mesg = cJSON_CreateObject();
        cJSON_AddStringToObject(Mesg,"Type","SignUp");
        cJSON_AddStringToObject(Mesg,"ID", in_txt[1].toStdString().c_str());
        cJSON_AddStringToObject(Mesg,"PW", in_txt[2].toStdString().c_str());
        char *JsonToSend = cJSON_Print(Mesg);
        cJSON_Delete(Mesg);
        if (send(socfd, JsonToSend, 512, NULL) < 0) {
            QMessageBox::critical(NULL, "Error", "Failed to Register!");
            return;
        }
        QThread::sleep(1);
        Mesg = cJSON_CreateObject();
        cJSON_AddStringToObject(Mesg,"Type","Exit");
        JsonToSend = cJSON_Print(Mesg);   //make the json as char*
        cJSON_Delete(Mesg);
        send(socfd, JsonToSend, 512, NULL);
        ::close(socfd);
        QMessageBox::information(NULL, "Congratulation", "Register successfully!");
    }
}

void game::startVSblackAI()
{
    delete Siri;
    Siri =NULL;
    playerside = 0;
    onlineGame = false;
    gameScene->clear();
    playOffline();
    addToScene(turnDisplay);
    addToScene(check);
    placeTheBoard();
    placePieces();
    AIsSide = 1;
    Siri = new stupid_AI(AIsSide);
}

void game::startVSwhiteAI()
{
    delete Siri;
    Siri =NULL;
    playerside = 1;
    gameScene->clear();
    onlineGame = false;
    playOffline();
    addToScene(turnDisplay);
    addToScene(check);
    placeTheBoard();
    placePieces();
    AIsSide = 0;
    Siri = new stupid_AI(AIsSide);
    AIsMove();
}

boardbox *game::getbox(int i, int j)
{
    boardbox *ret = NULL;
    if (board != NULL)
        ret= board->getbox(i,j);
    if (ret)
        return ret;
    else
        return NULL;
}

void game::pickUpPieces(Piece *P)
{
    if (/*card->getOwner() == getWhosTurn() &&*/ P != NULL)
    {
        piece_to_placed = P;
        originalPos = P->pos();
    }
}

void game::placePieces()
{
    if (board != NULL)
        board->startup();
}

void game::mainmenu()
{
    delete Siri;
    Siri =NULL;
    playerside = 0;
    onlineGame = false;
    //Create the title
    gameScene->clear();
    QGraphicsTextItem *titleText = new QGraphicsTextItem("Online Chess Game");
    QFont titleFont("arial" , 50);
    titleText->setFont( titleFont);
    int xPos = width()/2 - titleText->boundingRect().width()/2;
    int yPos = 100;
    titleText->setPos(xPos,yPos);
    addToScene(titleText);

    button * registerButton = new button("Register Account");
    xPos = width()/2 - registerButton->boundingRect().width()/2;
    yPos = 225;
    registerButton->setPos(xPos,yPos);
    connect(registerButton, SIGNAL(clicked()) , this , SLOT(register_user()));
    addToScene(registerButton);

    button * playButton = new button("Play vs human");
    int pxPos = width()/2 - playButton->boundingRect().width()/2;
    int pyPos = 300;
    playButton->setPos(pxPos,pyPos);
    connect(playButton,SIGNAL(clicked()) , this , SLOT(start()));
    addToScene(playButton);

    button * BlackAIButton = new button("Play as White vs AI");
    int pxPos1 = width()/2 - BlackAIButton->boundingRect().width()/2;
    int pyPos1 = 375;
    BlackAIButton->setPos(pxPos1,pyPos1);
    connect(BlackAIButton,SIGNAL(clicked()) , this , SLOT(startVSblackAI()));
    addToScene(BlackAIButton);

    button * WhiteAIButton = new button("Play as Black vs AI");
    int wxPos1 = width()/2 - WhiteAIButton->boundingRect().width()/2;
    int wyPos1 = 450;
    WhiteAIButton->setPos(wxPos1,wyPos1);
    connect(WhiteAIButton,SIGNAL(clicked()) , this , SLOT(startVSwhiteAI()));
    addToScene(WhiteAIButton);

    button * Playonline = new button("Play Online");
    int oxPos1 = width()/2 - WhiteAIButton->boundingRect().width()/2;
    int oyPos1 = 525;
    Playonline->setPos(oxPos1,oyPos1);
    connect(Playonline, SIGNAL(clicked()) , this , SLOT(openGameLobby()));
    addToScene(Playonline);

    //Create Quit Button
    button * quitButton = new button("Exit");
    int qxPos = width()/2 - quitButton->boundingRect().width()/2;
    int qyPos = 600;
    quitButton->setPos(qxPos,qyPos);
    connect(quitButton, SIGNAL(clicked()),this,SLOT(close()));
    addToScene(quitButton);
    //listG.append(quitButton);
}

void game::openGameLobby()
{
    delete Lobby;
    Lobby = NULL;
    Lobby = new gameLobby();
    if (!Lobby->connectError)
    {
        Lobby->show();
        hide();
    }
    else
    {
        delete Lobby;
        Lobby = NULL;
    }
}

void game::backToLobby()
{
    playerside = 0;
    onlineGame = true;
    mainmenu();
    hide();
    Lobby->yourSide = -1;
    Lobby->inRooms = false;
    Lobby->waiting = false;
    Lobby->host = false;
    Lobby->backToLobby();
}

void game::SHOW()
{
    show();
}


void game::mouseMoveEvent(QMouseEvent *event)
{
    if (piece_to_placed){
       piece_to_placed->setPos((event->pos().x()-50),(event->pos().y()-50));
       piece_to_placed->setZValue(10);
    }
    QGraphicsView::mouseMoveEvent(event);
}

void game::mouseReleaseEvent(QMouseEvent *event)
{
    int startX = event->pos().x();
    int startY = event->pos().y();
    QGraphicsView::mouseReleaseEvent(event);
    int finalX = startX/100*100;
    int finalY = (startY-50)/100*100;
    int x = finalX/100-3;
    int y = finalY/100;
    if (playerside)
    {
        x = 7-x;
        y = 7-y;
    }
    //qDebug()<<"x: "<<x<<" y: "<<y;
    finalY += 50;
    //if (piece_to_placed && piece_to_placed->canmove(finalPos.x(),finalPos.y())){
   //     piece_to_placed->setPos(finalPos);
    //    piece_to_placed = NULL;
    // } // A bad bug example here, if this function doest run, the piece is still not null.

    if (piece_to_placed)
    {
        int Pieceside = piece_to_placed->getside();
        if (startX < 300 || startX >= 1100)
        {
            piece_to_placed->setPos(originalPos);
            piece_to_placed->setZValue(piece_to_placed->origin_zValue);
            piece_to_placed = NULL;
            return;
        }
        else if (startY < 50 ||startY >= 850)
        {
            piece_to_placed->setPos(originalPos);
            piece_to_placed->setZValue(piece_to_placed->origin_zValue);
            piece_to_placed = NULL;
            return;
        }
        boardbox *targetBox = getbox(x,y);
        if(piece_to_placed->pawnAttack(x,y) && targetBox->hasPiece() && targetBox->getpiece()->getside() != piece_to_placed->getside())
        {
            piece_to_placed->tryToMoveTo(x,y);
            if (board->checkCanCheck() == turn)
            {
                board->gobackThinking();
                piece_to_placed->setPos(originalPos);
                piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                piece_to_placed = NULL;
                return;
            }
            else if (board->checkCanCheck() == (!turn))
                checking = true;
            else
                checking = false;
            board->gobackThinking();

            int diediediedie = targetBox->getpiece()->die();
            if (diediediedie+1)
            {
                qDebug() << "Game over!";
                gameOver(diediediedie);
                if (onlineGame)
                {
                    if(!Lobby->sendMove(piece_to_placed->getCol(),piece_to_placed->getRow(),x,y))
                    {
                        piece_to_placed->setPos(originalPos);
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        return;
                    }

                }
                piece_to_placed->moveTo(x,y);
                piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                piece_to_placed = NULL;
                return;
            }
            if (onlineGame)
            {
                if(!Lobby->sendMove(piece_to_placed->getCol(),piece_to_placed->getRow(),x,y))
                {
                    piece_to_placed->setPos(originalPos);
                    piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                    piece_to_placed = NULL;
                    return;
                }

            }
            piece_to_placed->moveTo(x,y);
            if (y == 0+Pieceside*7)
            {
                Piece *newPiece = new queen(Pieceside,x,y);
                newPiece->setPos(finalX, finalY);
                board->appendPieces(newPiece);
                piece_to_placed->setdie(true);
                gameScene->removeItem(piece_to_placed);
                addToScene(newPiece);
                piece_to_placed = newPiece;
                if (board->checkCanCheck() == (!turn))
                    checking = true;
                else
                    checking = false;
                if (checking)
                {
                    if(!check->isVisible())
                        check->setVisible(true);
                    if(!CanYouMove(!turn))
                    {
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        gameOver(!turn);
                        return;
                    }

                }
            }
            piece_to_placed->setZValue(piece_to_placed->origin_zValue);
            piece_to_placed = NULL;
            if (checking)
            {
                if(!check->isVisible())
                    check->setVisible(true);
                if(!CanYouMove(!turn))
                {
                    gameOver(!turn);
                    return;
                }
            }
            else
                check->setVisible(false);
            changeTurn();
            return;
        }
        else if(piece_to_placed->canmove(x,y))
        {
            piece_to_placed->tryToMoveTo(x,y);
            if (board->checkCanCheck() == turn)
            {
                board->gobackThinking();
                piece_to_placed->setPos(originalPos);
                piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                piece_to_placed = NULL;
                return;
            }
            else if (board->checkCanCheck() == (!turn))
                checking = true;
            else
                checking = false;
            board->gobackThinking();
            if (targetBox->hasPiece())
            {
                if (targetBox->getpiece()->getside() == piece_to_placed->getside())
                {
                    piece_to_placed->setPos(originalPos);
                    piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                    piece_to_placed = NULL;
                    return;
                }
                else
                {
                    int diediediedie = targetBox->getpiece()->die();
                    //qDebug() << diediediedie;
                    if (diediediedie+1)
                    {
                        qDebug() << "Game over!";
                        gameOver(diediediedie);
                        if (onlineGame)
                        {
                            if(!Lobby->sendMove(piece_to_placed->getCol(),piece_to_placed->getRow(),x,y))
                            {
                                piece_to_placed->setPos(originalPos);
                                piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                                piece_to_placed = NULL;
                                return;
                            }

                        }
                        piece_to_placed->moveTo(x,y);
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        return;
                    }
                }
            }
            if (onlineGame)
            {
                if (piece_to_placed->getType() == 9 && piece_to_placed->getCol() == 4 && (x == 2 || x == 6))
                {
                    int castling = -1;
                    if (x == 2)
                        castling = 0;
                    else
                        castling = 7;
                    if(!Lobby->sendMove(piece_to_placed->getCol(),piece_to_placed->getRow(),x,y, castling))
                    {
                        piece_to_placed->setPos(originalPos);
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        return;
                    }
                }
                else
                {
                    if(!Lobby->sendMove(piece_to_placed->getCol(),piece_to_placed->getRow(),x,y))
                    {
                        piece_to_placed->setPos(originalPos);
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        return;
                    }
                }
            }
            piece_to_placed->moveTo(x,y);
            if (y == 0+Pieceside*7 && piece_to_placed->getType()==4)
            {
                Piece *newPiece = new queen(Pieceside,x,y);
                newPiece->setPos(finalX, finalY);
                board->appendPieces(newPiece);
                piece_to_placed->setdie(true);
                gameScene->removeItem(piece_to_placed);
                addToScene(newPiece);
                piece_to_placed = newPiece;
                if (board->checkCanCheck() == (!turn))
                    checking = true;
                else
                    checking = false;
                if (checking)
                {
                    if(!check->isVisible())
                        check->setVisible(true);
                    if(!CanYouMove(!turn))
                    {
                        piece_to_placed->setZValue(piece_to_placed->origin_zValue);
                        piece_to_placed = NULL;
                        gameOver(!turn);
                        return;
                    }

                }
            }
            piece_to_placed->setZValue(piece_to_placed->origin_zValue);
            piece_to_placed = NULL;
            if (checking)
            {
                if(!check->isVisible())
                    check->setVisible(true);
                if(!CanYouMove(!turn))
                {
                    gameOver(!turn);
                    return;
                }
            }
            else
                check->setVisible(false);
            changeTurn();
            return;
            //this line was a interesting line here, I deleted it, couldn't figure out what happened: QGraphicsView::mouseReleaseEvent(event);
        }
        else
        {
            piece_to_placed->setPos(originalPos);
            //qDebug() << "o:" <<piece_to_placed->location[0] << ","<<piece_to_placed->location[1];
            //qDebug() << "f:" <<finalX/100-3 << ","<<(finalY-50)/100;
            //qDebug() << "f2:" <<x << ","<<y;
            piece_to_placed->setZValue(piece_to_placed->origin_zValue);
            piece_to_placed = NULL;
            return;
        }
     }
    piece_to_placed = NULL;
}


int game::getTurn()
{
    return turn;
}

void game::setTurn(int i)
{
    turn = i;
}

void game::AIsMove()
{
    delay();

    std::shared_ptr<possible_boxNpiece> lol = board->findGoodMovesOneTrun(AIsSide, Siri);
    //for random move

    //board->findPossibleMove(AIsSide); for random move
    //possible_boxNpiece *lol = Siri->getMove( &(board->possible_boxNpiece_Black));
    if(lol == NULL)
    {
        qDebug() << "Game over!";
        gameOver(AIsSide); //white is checked
        return;
    }
    if(lol->possibleMove->hasPiece())
    {
        Piece* willdie = lol->possibleMove->getpiece();
        int diediediedie = willdie->die();
            //qDebug() << diediediedie;
            if (diediediedie+1)
            {
                qDebug() << "Game over!";
                gameOver(diediediedie);
                lol->targetPiece->moveTo(lol->BoxCol,lol->BoxRow);
                lol = NULL;
                return;
            }
    }
    lol->targetPiece->moveTo(lol->BoxCol,lol->BoxRow);

    if (board->checkCanCheck() == (!AIsSide))
        checking = true;
    else
        checking = false;
    if (checking)
    {
        if(!check->isVisible())
            check->setVisible(true);
        if(!CanYouMove(!AIsSide))
        {
            gameOver(!AIsSide);
            return;
        }

    }
    else
        check->setVisible(false);
    changeTurn();
}

void game::changeTurn()
{
    if(turn)
    {
        turn = 0;
        turnDisplay->setPlainText("Turn : WHITE");
        if (turn == AIsSide)
            AIsMove();
    }
    else
    {
        turn = 1;
        turnDisplay->setPlainText("Turn : BLACK");
        if (turn == AIsSide)
            AIsMove();
    }
}

void game::playOffline()
{
    piece_to_placed = NULL; // always initiate and delete it first.

    turn = 0; //0 is white and 1 is black
    turnDisplay = new QGraphicsTextItem();
    turnDisplay->setPos(width()/2-100,10);
    turnDisplay->setZValue(1);
    turnDisplay->setDefaultTextColor(Qt::white);
    turnDisplay->setFont(QFont("",20));
    turnDisplay->setPlainText("Turn : WHITE");

    check = new QGraphicsTextItem();
    check->setPos(width()/2-100,850);
    check->setZValue(5);
    check->setDefaultTextColor(Qt::red);
    check->setFont(QFont("",45));
    check->setPlainText("CHECK!");
    check->setVisible(false);

}

void game::SetGamecolor()
{
    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    QColor gray(Qt::gray);
    brush.setColor(gray);
    setBackgroundBrush(brush);
}

void game::gameOver(int color)
{
    turn = 2;


    QGraphicsRectItem *rect(new QGraphicsRectItem());
    rect->setRect(0,0,450,300);
    QBrush Abrush;
    Abrush.setStyle(Qt::SolidPattern);
    Abrush.setColor(QColor(199, 231, 253));
    rect->setBrush(Abrush);
    rect->setZValue(4);
    int pxPos = width()/2 - rect->boundingRect().width()/2;
    int pyPos = 250;
    rect->setPos(pxPos,pyPos);
    addToScene(rect);

    QGraphicsTextItem *whowin;
    if (color == 0)
        whowin = new QGraphicsTextItem("Black Wins!");
    else
        whowin = new QGraphicsTextItem("White Wins!");
    QFont titleFont("arial" , 30);
    whowin->setFont( titleFont);
    int axPos = width()/2 - whowin->boundingRect().width()/2;
    int ayPos = 300;
    whowin->setPos(axPos,ayPos);
    whowin->setZValue(5);
    addToScene(whowin);

    button * replayButton = new button("Play Again");
    int qxPos = width()/2 - rect->boundingRect().width()/2 + 10;
    int qyPos = 400;
    replayButton->setPos(qxPos,qyPos);
    replayButton->setZValue(5);
    if (onlineGame)
        connect(replayButton,SIGNAL(clicked()) , Lobby , SLOT(I_wannaPlayAgain()));
    else if (AIsSide == -1)
        connect(replayButton,SIGNAL(clicked()) , this , SLOT(start()));
    else
        connect(replayButton,SIGNAL(clicked()) , this , SLOT(startVSblackAI()));
    addToScene(replayButton);

    //Create Quit Button
    if (onlineGame)
    {
        button * ReturnButton = new button("Return to the Lobby");
        int rxPos = width()/2 + 15;
        int ryPos = 400;
        ReturnButton->setPos(rxPos,ryPos);
        ReturnButton->setZValue(5);
        //TO DO:
        // send message :exit
        connect(ReturnButton, SIGNAL(clicked()),this,SLOT(backToLobby()));
        addToScene(ReturnButton);
    }
    else
    {
        button * ReturnButton = new button("Return to Mainmenu");
        int rxPos = width()/2 + 15;
        int ryPos = 400;
        ReturnButton->setPos(rxPos,ryPos);
        ReturnButton->setZValue(5);
        connect(ReturnButton, SIGNAL(clicked()),this,SLOT(mainmenu()));
        addToScene(ReturnButton);
    }
}

void game::delay()
{
    QTime dieTime= QTime::currentTime().addSecs(1);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

bool game::CanYouMove(int yourturn)
{
    std::vector<std::vector<int>> map = board->createloaclmap();
    findallmovess *yourturnsmove = new findallmovess(yourturn, map);
    if (yourturnsmove->allmoves.length() == 0)
    {
        delete yourturnsmove;
        return false;
    }
    else
    {
        delete yourturnsmove;
        return true;
    }
}

void game::receiveMove(onlineMove* move)
{
    int fromX = move->fromX;
    int fromY = move->fromY;
    int x = move->ToX;
    int y = move->ToY;
    int Cast = move->Castling;
    delete move;
    boardbox *Piecesbox = board->getbox(fromX,fromY);

    boardbox *targetBox = board->getbox(x,y);
    Piece *targetPiece = Piecesbox->getpiece();

    if (targetBox->hasPiece())
    {
        int diediediedie = targetBox->getpiece()->die();
        if (diediediedie+1)
        {
            qDebug() << "Game over!";
            gameOver(diediediedie);
            targetPiece->moveTo(x,y);
            return;
        }
    }

    if (Cast >= 0)
    {
        Piece * rook = board->getbox(Cast,fromY)->getpiece();
        int rookX;
        if (Cast == 0)
            rookX = 3;
        else
            rookX = 5;
        rook->moveTo(rookX, fromY);
    }
    targetPiece->moveTo(x,y);

    if (board->checkCanCheck() == (!turn))
        checking = true;
    else
        checking = false;

    int Pieceside = targetPiece->getside();
    if (y == 0+Pieceside*7 && targetPiece->getType() == 4)
    {
        Piece *newPiece = new queen(Pieceside,x,y);
        newPiece->setPos(targetPiece->pos().x(), targetPiece->pos().y());
        board->appendPieces(newPiece);
        targetPiece->setdie(true);
        gameScene->removeItem(targetPiece);
        addToScene(newPiece);
        if (board->checkCanCheck() == (!turn))
            checking = true;
        else
            checking = false;
    }
    if (checking)
    {
        if(!check->isVisible())
            check->setVisible(true);
        if(!CanYouMove(!turn))
        {
            gameOver(!turn);
            return;
        }
    }
    else
        check->setVisible(false);
    changeTurn();
}

void game::closeEvent(QCloseEvent *event)
{
    if (Lobby)
        Lobby->close();
}

void game::palyAsWhiteOnline()
{
    delete Siri;
    Siri =NULL;
    AIsSide = -1;
    playerside = 0;
    onlineGame = true;
    gameScene->clear();
    playOffline();
    addToScene(turnDisplay);
    addToScene(check);
    placeTheBoard();
    placePieces();
}

void game::palyAsBlackOnline()
{
    delete Siri;
    Siri =NULL;
    AIsSide = -1;
    playerside = 1;
    onlineGame = true;
    gameScene->clear();
    playOffline();
    addToScene(turnDisplay);
    addToScene(check);
    placeTheBoard();
    placePieces();
}


