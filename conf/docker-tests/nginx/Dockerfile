FROM debian:bullseye

RUN apt-get update -y && apt-get install -y \
	nginx \
	nano

RUN rm -rf /etc/nginx/sites-enabled/default /etc/nginx/sites-available/default

# the alpha order is important, since the first is server in absence of default_server
RUN mkdir /var/www/farshad.com
RUN mkdir /var/www/navid.com
RUN mkdir /var/www/nico.com

COPY farshad.com /var/www/farshad.com
COPY navid.com /var/www/navid.com
COPY nico.com /var/www/nico.com

# confs
COPY conf/farshad.conf /etc/nginx/sites-available/farshad.conf
COPY conf/navid.conf /etc/nginx/sites-available/navid.conf
COPY conf/nico.conf /etc/nginx/sites-available/nico.conf

RUN ln -s /etc/nginx/sites-available/farshad.conf /etc/nginx/sites-enabled/farshad.conf
RUN ln -s /etc/nginx/sites-available/navid.conf /etc/nginx/sites-enabled/navid.conf
RUN ln -s /etc/nginx/sites-available/nico.conf /etc/nginx/sites-enabled/nico.conf

# CMD [ "tail", "-f" ]
ENTRYPOINT [ "/usr/sbin/nginx" ]
CMD [ "-g", "daemon off;" ]

# docker stop nginx && docker rm nginx
# docker rmi nginx client
