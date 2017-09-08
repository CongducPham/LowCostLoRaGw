In the `/etc/sudoers` file, 

```sh
sudo visudo
```

add the following at the end 

```sh
www-data ALL=(ALL) NOPASSWD: /var/www/html/images/libs/sh/move_img.sh
```