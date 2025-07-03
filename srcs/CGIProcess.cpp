/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIProcess.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mgalvez <mgalvez@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/19 14:57:00 by mgalvez           #+#    #+#             */
/*   Updated: 2025/06/02 22:03:58 by mgalvez          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGIProcess.hpp"

int					*CGIProcess::getStatus()
{
	return (&this->_status);
}

ssize_t				CGIProcess::getBytesRead() const
{
	return (this->_bytes_read);
}

int					CGIProcess::getTempFileFd() const
{
	return (this->_temp_file_fd);
}

std::string const	&CGIProcess::getTempFileName() const
{
	return (this->_temp_file_name);
}

int					CGIProcess::getFdOut() const
{
	return (this->_fd_out);
}

int					CGIProcess::getFdIn() const
{
	return (this->_fd_in);
}

pid_t				CGIProcess::getPid() const
{
	return(this->_pid);
}

void				CGIProcess::setBytesRead(ssize_t bytes_read)
{
	this->_bytes_read = bytes_read;
}

void				CGIProcess::setTempFileFd(int temp_file_fd)
{
	this->_temp_file_fd = temp_file_fd;
}

void				CGIProcess::setTempFileName(std::string &temp_file_name)
{
	this->_temp_file_name = temp_file_name;
}

void				CGIProcess::setFdOut(int fd_out)
{
	this->_fd_out = fd_out;
}

void				CGIProcess::setFdIn(int fd_in)
{
	this->_fd_in = fd_in;
}

CGIProcess::~CGIProcess()
{
	if (this->_fd_in)
		close(this->_fd_in);
	if (this->_fd_out)
		close(this->_fd_out);
	if (this->_temp_file_fd)
		close(this->_temp_file_fd);
	unlink(this->_temp_file_name.c_str());
}

CGIProcess::CGIProcess(pid_t const pid, int const fd_in, int const fd_out):
						_pid(pid),
						_fd_in(fd_in),
						_fd_out(fd_out)
{
	std::ifstream	random("/dev/urandom");
	std::string		name;
	size_t	idx = 0;
	while (idx < 25)
	{
		char	c = (static_cast<size_t>(random.get()) % 74) + 48;
		name += c;
		++idx;
	}
	this->_temp_file_name = ".tmp/" + name + ".html";
	this->_bytes_read = 0;
	this->_status = -1;
	mkdir(".tmp", 0755);
	this->_temp_file_fd = open(this->_temp_file_name.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	fcntl(fd_out, F_SETFL, O_NONBLOCK);
}
