//
//  SocketException.cpp
//  ParallelerServerC++
//
//  Created by Dennis Halter on 09.12.19.
//  Copyright © 2019 Dennis Halter. All rights reserved.
//

#include "SocketException.hpp"

SocketException::SocketException(std::string error){
    _error = error;
}

std::string SocketException::show(void){
    return _error;
}