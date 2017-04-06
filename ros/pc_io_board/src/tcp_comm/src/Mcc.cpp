//
// Created by Gonzalo Pelós on 2/8/17.
//

#include "../include/tcp_comm/Mcc.h"
// #include <mutex>/
#include <pthread.h>

#include "../include/tcp_comm/Tcp_Controller.h"

// using namespace std;

char ok_str[] = "OK";
char error_str[] = "ERROR";
char parseerror_str[] = "PARSEERROR";
char exeerror_str[] = "EXEERROR";

enum ProtocolStates
{
    MESSAGE,
    TPID,
    OPCODE,
    DATA,
    DATA_ITEM,
    END,
    NONE
};

enum ProtocolStates protocol_state = MESSAGE;
char embuf[255];
char outbuf[255];
char mcc_message[255];
bool mcc_message_ready;

EmBdecode decoder(embuf, sizeof(embuf));
uint8_t tpid;
uint8_t opcode;

pthread_mutex_t serial_mcc_frame_lock;

/***********************
 * EmBencode Operations
 ***********************/
void EmBencode::lock()
{
    pthread_mutex_lock(&serial_mcc_frame_lock);
    // serial_mcc_frame_lock.lock();
}

void EmBencode::unlock()
{
    pthread_mutex_unlock(&serial_mcc_frame_lock);
    // serial_mcc_frame_lock.unlock();
}

void EmBencode::PushChar(char ch)
{
    if (strlen(outbuf) < 255)
    {
        outbuf[strlen(outbuf)] = ch;
        if (ch == '\n')
        {
            memset(mcc_message, 0, 255);
            strcpy(mcc_message, outbuf);
            memset(outbuf, 0, 255);
        }
    }
}

/**************************/

Mcc::Mcc()
{
    incomming_params_count = 0;
    n_poll_callbacks = 0;
    n_opcode_callbacks = 0;
    memset(outbuf, 0, 255);
}

Mcc::~Mcc()
{
    // TODO Auto-generated destructor stub
}

void Mcc::process_incomming(char ch)
{
    if (ch == '\n')
    {
        decoder.reset();
        tpid = opcode = -1;
        incomming_params_count = 0;
        return;
    }

    uint8_t bytes = decoder.process(ch);
    if (bytes > 0)
    {
        protocol_state = MESSAGE;
        for (;;)
        {
            uint8_t token = decoder.nextToken();
            if (token == EmBdecode::T_END)
            {
                //decoder.reset();
                break;
            }

            switch (protocol_state)
            {
            case MESSAGE:
                if (token == EmBdecode::T_LIST)
                {
                    //pc.putc('A');
                    protocol_state = TPID;
                }
                else
                {
                    //pc.putc('X');
                    send_parse_error_message();
                    protocol_state = NONE;
                }
                break;
            case TPID:
                if (token == EmBdecode::T_NUMBER)
                {
                    //pc.('B');
                    tpid = decoder.asNumber();
                    protocol_state = OPCODE;
                }
                else
                {
                    //pc.putc('X');
                    send_parse_error_message();
                    protocol_state = NONE;
                }
                break;
            case OPCODE:
                if (token == EmBdecode::T_NUMBER)
                {
                    //pc.putc('C');
                    opcode = decoder.asNumber();
                    protocol_state = DATA;
                }
                else
                {
                    //pc.putc('X');
                    send_parse_error_message();
                    protocol_state = NONE;
                }
                break;
            case DATA:
                if (token == EmBdecode::T_LIST)
                {
                    //pc.putc('I');
                    protocol_state = DATA_ITEM;
                    incomming_params_count = 0;
                }
                else
                {
                    //pc.putc('X');
                    send_parse_error_message();
                    protocol_state = NONE;
                }
                break;
            case DATA_ITEM:
                if (token == EmBdecode::T_STRING)
                {
                    //pc.putc('S');
                    uint8_t data_length;
                    incomming_params_s[incomming_params_count] = decoder.asString(&data_length);
                    incomming_params_n[incomming_params_count] = data_length;
                    incomming_params_count++;
                    if (incomming_params_count == MAX_PARAMS)
                    {
                        //pc.putc('X');
                        send_parse_error_message();
                        protocol_state = NONE;
                    }
                }
                else if (token == EmBdecode::T_NUMBER)
                {
                    //pc.putc('N');
                    incomming_params_s[incomming_params_count] = NULL;
                    incomming_params_n[incomming_params_count] = decoder.asNumber();
                    incomming_params_count++;
                    if (incomming_params_count == MAX_PARAMS)
                    {
                        //pc.putc('Y');
                        send_parse_error_message();
                        protocol_state = NONE;
                    }
                }
                else if (token == EmBdecode::T_POP)
                {
                    //pc.putc('H');
                    protocol_state = END;
                }
                else
                {
                    send_parse_error_message();
                    protocol_state = NONE;
                }
                break;
            case END:
                if (token == EmBdecode::T_POP) {
                    // if (tpid < MAX_PIDS && tpid < Mcc::n_opcode_callbacks && opcode < (Mcc::opcode_callbacks[tpid]).n_callbacks)
                    // {
                        //pc.putc('!');
                        // int ret = Mcc::opcode_callbacks[tpid].callbacks[opcode](tpid, opcode);
                        int ret = Mcc::opcode_callbacks[0].callbacks[0](tpid, opcode, create_mcc_params_from_incomming_data());
                        if (ret != 1)
                        {
                            send_execution_error_message(tpid, opcode, ret);
                            //protocol_state = NONE;
                        }
                    // }
                    //decoder.reset();
                    protocol_state = NONE;
                    //pc.putc('E');
                } else {
                    //pc.putc('W');
                    send_parse_error_message();
                    protocol_state = NONE;
                }
                break;
            case NONE:
                break;
            }
        }
        tpid = opcode = -1;
        incomming_params_count = 0;

        decoder.reset();
    }
}

void Mcc::send_parse_error_message()
{
    encoder.startFrame();
    encoder.push(ADMIN_PID);
    encoder.push(OPCODE_REPORT);
    encoder.startList();
    encoder.push(parseerror_str, sizeof(parseerror_str) - 1);

    encoder.push(tpid);
    encoder.push(opcode);
    encoder.push(protocol_state);

    for (int i = 0; i < incomming_params_count; ++i)
    {
        if (incomming_params_s[i] != NULL)
        {
            encoder.push(incomming_params_s[i], incomming_params_n[i]);
        }
        else
        {
            encoder.push(incomming_params_n[i]);
        }
    }

    encoder.endList();
    encoder.endFrame();
}

void Mcc::send_execution_error_message(int tpid, int opcode, int errcode)
{
    encoder.startFrame();
    encoder.push(ADMIN_PID);
    encoder.push(OPCODE_REPORT);
    encoder.startList();
    encoder.push(exeerror_str, sizeof(exeerror_str) - 1);
    encoder.push(errcode);
    encoder.push(tpid);
    encoder.push(opcode);
    for (int i = 0; i < incomming_params_count; ++i)
    {
        if (incomming_params_s[i] != NULL)
        {
            encoder.push(incomming_params_s[i], incomming_params_n[i]);
        }
        else
        {
            encoder.push(incomming_params_n[i]);
        }
    }
    encoder.endList();
    encoder.endFrame();
}

int Mcc::register_poll_callback(PollCallback cb)
{
    int result = 0;

    if (Mcc::n_poll_callbacks >= MAX_POLL_CALLBACKS)
    {
        return -1;
    }
    Mcc::poll_callbacks[Mcc::n_poll_callbacks++] = cb;
    return Mcc::n_poll_callbacks;

    return result;
}

void Mcc::unregister_poll_callback(int n)
{
    if (n < Mcc::n_poll_callbacks - 1)
    {
        Mcc::poll_callbacks[n] = Mcc::poll_callbacks[Mcc::n_poll_callbacks - 1];
    }
}

int Mcc::register_opcode_callbacks(OpcodeCallback *opcode_callbacks, uint8_t opcodes_count)
{
    int result = -1;
    if (Mcc::n_opcode_callbacks < MAX_PIDS)
    {
        uint8_t pid = Mcc::n_opcode_callbacks++;
        Mcc::opcode_callbacks[pid].callbacks = opcode_callbacks;
        Mcc::opcode_callbacks[pid].n_callbacks = opcodes_count;
        result = pid;
    }
    return result;
}

void Mcc::send_message(int spid, uint8_t opcode, const char *data, uint8_t data_length)
{
    encoder.startFrame();
    encoder.push(spid);
    encoder.push(opcode);
    encoder.startList();
    if (data != NULL)
    {
        encoder.push(data, data_length);
    }
    encoder.endList();
    encoder.endFrame();
}

// void Mcc::tick() {
//     Mcc::process_incomming();
//     for (int i=0; i<Mcc::n_poll_callbacks; ++i){
//         Mcc::poll_callbacks[i]();
//     }
// }

char *Mcc::create_mcc_message(int spid, uint8_t opcode, const char *params_bencoded)
{
    int params_bencoded_length = -1;
    encoder.startFrame();
    memset(mcc_message, 0, sizeof(mcc_message));
    encoder.push(spid);
    encoder.push(opcode);
    params_bencoded_length = encoder.append_data(params_bencoded);
    if (params_bencoded_length > 0)
    {
        encoder.endFrame();
    }

    return mcc_message;
}

char* Mcc::create_mcc_params_from_incomming_data() {
  encoder.startFrame();
  memset(mcc_message, 0, sizeof(mcc_message));
    for (int i=0;i< incomming_params_count;++i) {
        if (incomming_params_s[i] != NULL) {
            encoder.push(incomming_params_s[i], incomming_params_n[i]);
            // printf("Mcc :: incomming_params_s received: %s\n", incomming_params_s[i]);
        } else {
            encoder.push(incomming_params_n[i]);
        }
    }
  encoder.endFrame();
  return mcc_message;
}
