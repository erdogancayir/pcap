/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   my_strncpy.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ecayir <erdogancayir>                      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 11:58:25 by ecayir            #+#    #+#             */
/*   Updated: 2025/05/11 11:58:26 by ecayir           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "my_libc.h"

char	*my_strncpy(char *dst, const char *src, size_t len)
{
	size_t i;

	i = 0;
	while ((src[i] != '\0') && (i < len))
	{
		dst[i] = src[i];
		i++;
	}
	while (i < len)
	{
		dst[i] = '\0';
		i++;
	}
	return (dst);
}
