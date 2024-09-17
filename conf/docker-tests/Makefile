all:
	docker compose up -d;

fclean:
	docker compose down;
	docker rmi nginx client;

# nginx: nginx-img
# 	docker run --detach --name $@ -p 8000:8000 $<;
# 	@if [ $$(docker ps -a --filter "status=running" | grep nginx | wc -l) -ne 0 ]; then \
# 		echo "$(G)* nginx is running$(W)"; \
# 	else \
# 		echo "$(R)* nginx exited$(W)"; \
# 	fi

# nginx-img:
# 	cd nginx && docker build -t $@ .

# nginx-rm:
# 	docker stop nginx;
# 	docker rm nginx;
# 	docker rmi nginx-img;

.PHONY: all fclean
