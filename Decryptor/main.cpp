#include <QCoreApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>

#include <ctime>
#include <cstdlib>
#include <iostream>
#include "encrypt.h"
#include "blockchain.h"

#define MEESAGE_SIZE        2                                       //NEW
char MessageToSend[] = {'t','y','r','e', 't', 't', '6', '.'};                 //NEW

QByteArray toHex(halfblock_t value)
{
    return "0x" + QString("%1").arg(value, 0, 16).rightJustified(HALF_SIZE/4, '0')
            .right(HALF_SIZE/4).toUpper().toUtf8();
}

using namespace std;
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    srand( time(nullptr) );

    halfblock_t r_keys[2*ROUND_NUM + 2];
    halfblock_t key[KEY_BLOCKS];

    char key_str[2 + HALF_SIZE/4];
    cout << "Enter key from microcontroller:\n";
    for(int i = 0; i < KEY_BLOCKS; i++) {
        cin >> key_str;
        if(key_str[0] == 'r' || key_str[0] == 'R') {
            i = -1;
            cout << "Restarting\n";
            continue;
        }
        key[i] = QString(key_str).toUInt(nullptr, 16);
        cout << "key[" << i << "] = ";
        cout << toHex(key[i]).constData() << ";\n";
    }

    gen_round_keys(key, r_keys);

    cout << "Key:\n";
    for(int i = 0; i < KEY_BLOCKS; i++) {
        cout << toHex(key[i]).constData() << " ";
    }
    cout << endl << endl;

//    qDebug() << "Round:" <<r_keys[0] << r_keys[1] <<
//            r_keys[2] << r_keys [3] << r_keys[48] << r_keys[49];

/*
//    cout << "Generated round keys:\n";
//    for(int i = 0; i <= ROUND_NUM; i++) {
//        cout << QString("%1:  ").arg(i).rightJustified(5, ' ').toUtf8().constData();
//        cout << toHex(r_keys[2*i]).constData() << "  ";
//        cout << toHex(r_keys[2*i + 1]).constData() << endl;
//    }
//    cout << endl;



//    halfblock_t encrypted[MEESAGE_SIZE];
//    halfblock_t decrypted[MEESAGE_SIZE];
//    halfblock_t * message=(halfblock_t *) MessageToSend;                          //NEW BEGIN
//    for(int i = 0; i < MEESAGE_SIZE/2; i++) {                         //Deleted
//        message[2*i] = (200*i)*30+1;
//        message[2*i + 1] = (567*(i%3)) + 1*i;

//        encrypt(message + 2*i, encrypted + 2*i, r_keys);
//        decrypt(decrypted+2*i, encrypted + 2*i, r_keys);
//    }

//    halfblock_t vector[2];

//#if HALF_SIZE==32
//        vector[0] = static_cast<uint32_t>( rand() << 16 | rand() );
//        vector[1] = static_cast<uint32_t>( rand() << 16 | rand() );
//#elif HALF_SIZE==64
//       vector[0] = static_cast<uint64_t>( rand() << 16 | rand() ) << 32;
//       vector[0] |= static_cast<uint64_t>( rand() << 16 | rand() );
//       vector[1] = static_cast<uint64_t>( rand() << 16 | rand() ) << 32;
//       vector[1] |= static_cast<uint64_t>( rand() << 16 | rand() );
//#endif

//    encryptblock(message,encrypted,MEESAGE_SIZE, vector, r_keys);
//    decryptblock(decrypted,encrypted,MEESAGE_SIZE, vector, r_keys);

//    cout << "Message send:\n";
//    for(int i = 0; i < MEESAGE_SIZE*4; i++)
//        cout << MessageToSend[i];
//    cout << endl;                                                    //NEW END

//    cout << "Bits -> Encrypted:\n";
//    for(int i = 0; i < MEESAGE_SIZE; i++) {
//        cout << toHex(message[i]).constData() << " -> ";
//        cout << toHex(encrypted[i]).constData() << endl;
//    }

//    cout << "Encrypted -> Decrypted:\n";
//    for(int i = 0; i < MEESAGE_SIZE; i++) {
//        cout << toHex(encrypted[i]).constData() << " -> ";
//        cout << toHex(decrypted[i]).constData() << endl;
//    }

//    char * MessageTaken;                                                      //NEW for controller
//    MessageTaken=(char *) decrypted;
//    cout << "Message taken:\n";
//    for(int i = 0; i < MEESAGE_SIZE*4; i++)
//        cout << MessageTaken[i];
//    cout << endl;
    */

    QSerialPort serial;
    serial.setPortName(QSerialPortInfo::availablePorts()[0].portName());
    serial.setBaudRate(9600);
    serial.setDataBits(QSerialPort::DataBits(8));
    serial.setParity(QSerialPort::Parity(0));
    serial.setStopBits(QSerialPort::StopBits(1));
    serial.setFlowControl(QSerialPort::FlowControl(0));

    cout << "Opening ";
    cout << serial.portName().toUtf8().constData() << "...\n";

    while(!serial.open(QIODevice::ReadWrite)) {
        cout << ".";
    }
    cout << "Success!\n";

    QByteArray arr;
    QByteArray recv;
    arr.clear();
    recv.clear();

    halfblock_t decr[100];
    halfblock_t *msg;
    halfblock_t vector[2];
#if HALF_SIZE==32
    vector[0] = 0x01234567;
    vector[1] = 0x89ABCDEF;
#elif HALF_SIZE==64
    vector[0] = 0x01234567 << 32;
    vector[0] |= 0x89ABCDEF;
    vector[1] = 0x01234567 << 32;
    vector[1] |= 0x89ABCDEF;
#endif

//#define MSG_LEN 56
    while(1) {
        if(serial.waitForReadyRead()) {
            arr.append( serial.readAll() );
        }
        while(arr.size() >= BLOCK_SIZE/8) {
            cout << arr.constData() << "\n";
            msg = (halfblock_t *)arr.constData();
            decryptblock(decr, msg, 2, vector, r_keys);
            //decryptblock(decr, msg, MSG_LEN*8/HALF_SIZE, vector, r_keys);
            recv.append((char*)decr);
            cout << recv.constData() << "\n";
            //cout << "arr = " << arr.constData() << ";\n";
            arr = arr.right(arr.size() - 8);
        }
    }


    return a.exec();
}
