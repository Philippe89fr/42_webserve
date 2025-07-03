/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sighandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgalvez <mgalvez@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 10:26:12 by mgalvez           #+#    #+#             */
/*   Updated: 2025/06/03 11:51:04 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

void	sighandler_child(void)
{
	struct sigaction	sigint_sigaction;
	struct sigaction	sigpipe_sigaction;

	memset(&sigint_sigaction, 0, sizeof(sigint_sigaction));
	memset(&sigpipe_sigaction, 0, sizeof(sigpipe_sigaction));
	sigint_sigaction.sa_handler = SIG_IGN;
	sigpipe_sigaction.sa_handler = SIG_DFL;
	sigaction(SIGINT, &sigint_sigaction, NULL);
	sigaction(SIGPIPE, &sigpipe_sigaction, NULL);
}

static void	sig_handler(int signal)
{
	extern int	g_sig_errno;

	g_sig_errno = signal;
	throw std::runtime_error("\r[Server] Closing signal recieved");
}

void	sighandler_init(void)
{
	struct sigaction	sigint_sigaction;
	struct sigaction	sigpipe_sigaction;

	memset(&sigint_sigaction, 0, sizeof(sigint_sigaction));
	memset(&sigpipe_sigaction, 0, sizeof(sigpipe_sigaction));
	sigint_sigaction.sa_handler = &sig_handler;
	sigpipe_sigaction.sa_handler = SIG_IGN;
	sigaction(SIGINT, &sigint_sigaction, NULL);
	sigaction(SIGPIPE, &sigpipe_sigaction, NULL);
}
