#include "Routing.h"

http_route_t routes[] = {
	{"/hello/.*", M_GET, sayHello },
	{"/admin/.*", M_GET, adminApi },
	{"/.*", 0xff, defaultHandler}, // Default route
};

static int routeMatches(char* pattern, char* path) {
	int meme;
	return re_match(pattern, path, &meme) != -1;
}

error_t doRoute(http_request_t* request, char** out, SIZE_T* outLen) {
	int i = 0;
	http_route_t* route = NULL;
	for (i = 0; i < sizeof(routes) / sizeof(http_route_t); i++) {
		route = &routes[i];
		if (routeMatches(route->pattern, request->path) &&
			(route->method == 0xff || request->method == route->method))
			break;
	}

	return route->handler(request, out, outLen);
}

error_t sayHello(http_request_t* request, char** out, SIZE_T* outLen) {
	http_response_t response = { 0 };
	error_t err = ERROR_UNINIT;

	buildResponse(&response, OK, request->contentType, "{\"hello\": 3.14}", sizeof("{\"hello\": 3.14}") - 1, 1);
	err = responseToString(&response, out, outLen);
	return err;
}

error_t adminApi(http_request_t* request, char** out, SIZE_T* outLen) {
	error_t err = ERROR_UNINIT;
	http_response_t response = { 0 };

	err = isAuthenticated(request);
	if (!IS_SUCCESS(err)) {
		printf("User unauthenticated with error %d\n", err);
		buildResponse(&response, UNAUTHORIZED, NULL, NULL, 0, 0);
		addHeader(&response, "WWW-Authenticate", "Basic realm=\"Access to the staging site\"");
		err = responseToString(&response, out, outLen);
		return err;
	}

	return sayHello(request, out, outLen);
}

error_t defaultHandler(http_request_t* request, char** out, SIZE_T* outLen) {
	error_t err = ERROR_UNINIT;
	SIZE_T fileSize = 0;
	BYTE* fileContent = NULL;
	http_response_t response = { 0 };

	char* path = request->path;
	if (!strcmp(path, "/"))
		path = "/index.html";

	if (!path)
		return ERROR_OK;

	if (!getFileContent(path, &fileContent, &fileSize)) {
		if (!getFileContent("/404/404.html", &fileContent, &fileSize))
			buildResponse(&response, NOT_FOUND, NULL, NULL, 0, 1);
		else
			buildResponse(&response, NOT_FOUND, "text/html", fileContent, fileSize, 1);
	}
	else {
		buildResponse(&response, OK, request->contentType, fileContent, fileSize, 1);
	}

	err = responseToString(&response, out, outLen);
	return err;
}