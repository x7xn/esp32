/**
* @file    miio_version.h
* @author  mashaoze
* @date    2017
* @par     Copyright (c):
*
*    Copyright 2017 MIoT,MI
*
*    Licensed under the Apache License, Version 2.0 (the "License");
*    you may not use this file except in compliance with the License.
*    You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
*    Unless required by applicable law or agreed to in writing, software
*    distributed under the License is distributed on an "AS IS" BASIS,
*    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*    See the License for the specific language governing permissions and
*    limitations under the License.
*/
#ifndef _MIIO_VERSION_H_
#define _MIIO_VERSION_H_

#define MIIO_VERSION_MAJOR					0
#define MIIO_VERSION_MINOR					0
#define MIIO_VERSION_PATCH					9

#define _VERSION_STR(v)						#v
#define MIIO_POINT_VERSION_STR(major,minor,path)	_VERSION_STR(major)"."_VERSION_STR(minor)"."_VERSION_STR(path)

#define MIIO_VERSION_STR					MIIO_POINT_VERSION_STR(MIIO_VERSION_MAJOR, MIIO_VERSION_MINOR, MIIO_VERSION_PATCH)

#endif
