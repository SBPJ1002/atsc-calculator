FROM php:8.2-apache

RUN apt-get update && apt-get install -y --no-install-recommends \
        libpq-dev \
        nodejs \
        npm \
    && docker-php-ext-install pdo pdo_pgsql sockets \
    && a2enmod rewrite headers \
    && rm -rf /var/lib/apt/lists/*

COPY webpage/ /var/www/html/

WORKDIR /var/www/html

RUN npm install && bash build.sh && rm -rf node_modules

RUN chown -R www-data:www-data /var/www/html

EXPOSE 80
