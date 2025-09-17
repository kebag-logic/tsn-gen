/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */
 

 #pragma once

#include <serializer_visitor.h>

/**
 * @brief This is the interface
 **/
class Serializer: public SerializerVisistor {
public:
	virtual ~Serializer() = default;

	/** 
	 * Abstracted class fo the visitor
	 */
	virtual int accept(SerializerVisistor& Visit) = 0;
};
