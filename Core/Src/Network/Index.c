/*
 * Index.c
 *
 *  Created on: Aug 15, 2023
 *      Author: joe.dobesh
 */

char HTML_index[] = {" \
<!DOCTYPE html> \
<!-- This is my Grad Project index page --> \
<html lang=\"en-US\"> \
	<head> \
		<meta charset=\"utf-8\" /> \
		<meta name=\"description\" content=\"Joe\'s Model Railroad Controller\" /> \
		<meta name=\"author\" content=\"Joe Dobesh\" /> \
		<meta http-equiv=\"refresh\" content=\"120\" /> \
		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" /> \
		<title>Railroad Engineer</title> \
		<style> \
			body {background-color:brown;} \
			h1 {text-align:center;} \
		</style> \
	</head> \
	<body> \
		<h1>Home Page</h1> \
		<hr /> \
		<p style=\"text-align:center;\"> \
			<button onclick=\"document.location=\'speedcontrol.html\'\">Speed Control Page</button> \
			<button onclick=\"document.location=\'loopcontrol.html\'\">Loop Control Page</button> \
		</p> \
		<hr /> \
	</body> \
</html> \
"};
