/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   errors.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nico <nico@student.42.fr>                  +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/20 15:12:05 by ncasteln          #+#    #+#             */
/*   Updated: 2024/09/09 13:52:14 by nico             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef __ERRORS_H__
# define __ERRORS_H__

enum file_err {
	E_TOOARGS = 110,
	E_INVFILE,
	E_ISDIR,
	E_FAIL
};

enum parse_err {
	E_ONLYTABS = 114,
	E_INVDIR,
	E_INVCONTEXT,
	E_INVIND,
	E_ENDDECL,
	E_EMPTYVAL,
	E_NOSERVER,
	E_NOLOCATION,
	E_CONTNAME,
	E_DUPLICATE
};

enum conf_err {
	CONF_SUCCESS = 0,
	E_TIMEOUT = 123,
	E_AUTOINDEX,
	E_CLIENTSIZE,
	E_HOST,
	E_PORT,
	E_METHOD,
	E_DOUBLEPORT,
	E_PORTNUM
};

enum server_err {
	ACCEPTING_FAILED = -1,
	NO_ERROR = 0,
	GET_ADDR_INFO_FAILED = 200,
	SOCKET_CREATION_FAILD,
	BIND_SOCKET_FAILED,
	LISTENING_FAILED,
	SOCKET_OPTIONS_FAILED,
	POLL_FAILED,
	EVENT_ERROR,
	CLIENTSOCKET_RECEIVE_FAILED,
	CGI_FAIL,
	HTTP_REQUEST_FAILED //navid_code
};

#endif /* __ERRORS_H__ */
