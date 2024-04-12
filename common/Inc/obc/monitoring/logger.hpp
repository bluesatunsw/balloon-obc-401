/* USER CODE BEGIN Header */
/*
 * 401 Ballon OBC
 * Copyright (C) 2023 BLUEsat and contributors
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */
/* USER CODE END Header */

#pragma once

namespace obc::monitoring {
    class LoggerStream {
        private:
        static LoggerStream instance;
        Log buffer[256];
        
            // TODO

        public:
        LoggerStream& operator<<(const char* message) {
            // TODO
            return *this;
        }
    };

    class Log {
        private:
        char message[256];
        int err;

        public:
        Log(const char* message, int err) : err(err) {
            // TODO
        }
    };

    class Logger {

        private:
        int subsys;
        LoggerStream stream;

        public:
        Logger(int subsys) : subsys(subsys) {
            // TODO
        }

        LoggerStream operator*() {
            // TODO Probably call steam.operator*()
        }

        /* 
        Log a message to the data logger
        @param message: The message to log
        @param err: The error code (See Confluence)
        @param subsys: The subsystem that the error occurred in
        */
        static void log(Log log) {
            // TODO
        }
    };
}  
