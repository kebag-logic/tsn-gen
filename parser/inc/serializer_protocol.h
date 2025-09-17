/*
 * SPDX-FileCopyrightText: 2025 Kebag-Logic (https://kebag-logic.com)
 * SPDX-FileCopyrightText: 2025 Alexandre Malki <alexandre.malki@kebag-logic.com>
 * SPDX-License-Identifier: MIT
 */
 

 #pragma once

#include <serializer.h>
#include <serializer_visitor-impl.h>


class SerializerProtocol: public Serializer {
public:
	SerializerProtocol() = default;

	int accept(SerializerVisitorImpl& t) {
		return 	t.visit(*this);
	}
private:

};
