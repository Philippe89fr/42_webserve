/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIProcess.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgalvez <mgalvez@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 14:49:37 by mgalvez           #+#    #+#             */
/*   Updated: 2025/06/02 22:03:55 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <fstream>
#include <unistd.h>

class	CGIProcess
{
public:
	CGIProcess(pid_t const pid, int const fd_in, int const fd_out);
	~CGIProcess();

	void				setFdIn(int fd_in);
	void				setFdOut(int fd_out);
	void				setTempFileName(std::string &temp_file_name);
	void				setTempFileFd(int temp_file_fd);
	void				setBytesRead(ssize_t bytes_read);
	pid_t				getPid() const;
	int					getFdIn() const;
	int					getFdOut() const;
	std::string const	&getTempFileName() const;
	int					getTempFileFd() const;
	ssize_t				getBytesRead() const;
	int					*getStatus();

private:
	pid_t const			_pid;
	int					_fd_in;
	int					_fd_out;
	std::string			_temp_file_name;
	int					_temp_file_fd;
	ssize_t				_bytes_read;
	int					_status;
};
