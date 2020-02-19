//
//  SocketException.hpp
//  ParallelerServerC++
//
//  Created by Dennis Halter on 09.12.19.
//  Copyright © 2019 Dennis Halter. All rights reserved.
//

#ifndef SocketException_hpp
#define SocketException_hpp

#include <iostream>


class SocketException{
private:
    std::string _error;
public:
    SocketException(std::string error);
    std::string show(void);
};


#endif /* SocketException_hpp */
