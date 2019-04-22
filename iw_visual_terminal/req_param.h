#ifndef REQ_PARAM_H
#define REQ_PARAM_H



class req_param
{
public:
    req_param();
    int level;
    int code;
    int value;
    int timeout;
    char send[20];
    int size;
};

#endif // REQ_PARAM_H
