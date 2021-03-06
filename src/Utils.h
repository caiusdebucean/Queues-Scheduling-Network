#ifndef UTILS_H_
#define UTILS_H_

enum PRIORITY {
    PREMIUM,
    STANDARD,
    ECONOMY
};

enum QUEUE {
    IS_NOT_EMPTY,
    IS_EMPTY
};

enum CAN_SEND_MESSAGE {
    ADEVARAT,
    FALS
};

enum TYPE {
    FUZZY_TIMER,
    NOT_FUZZY_TIMER,
    MULTIPLE_PACKETS_FROM_ONE_MSG,
    MULTIPLE_PACKETS_SEQUENCE
};

enum ERROR {
    IS_NOT_ERROR,
    IS_ERROR
};
#endif /* UTILS_H_ */
