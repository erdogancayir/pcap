/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   my_libc.h                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecayir <erdogancayir>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/10 20:36:12 by ecayir            #+#    #+#             */
/*   Updated: 2025/05/10 20:36:14 by ecayir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MY_LIBC_H
#define MY_LIBC_H

# include <unistd.h>
# include <stdlib.h>

void	*my_memset(void *b, int c, size_t len);
int     my_strcmp(const char *s1, const char *s2);

#endif
