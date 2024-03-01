/*
 * loopcontrol.c
 *
 *  Created on: Aug 15, 2023
 *      Author: joe.dobesh
 */

char HTML_loopcontrol[] = {" \
<!DOCTYPE html> \
<!-- This is my Grad Project Track Loop Control Page --> \
<html lang=\"en-US\"> \
	<head> \
		<meta charset=\"utf-8\" /> \
		<meta name=\"description\" content=\"Track Loop Control\" /> \
		<meta name=\"author\" content=\"Joe Dobesh\" /> \
		<meta http-equiv=\"refresh\" content=\"120\" /> \
		<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" /> \
		<title>Loop Control</title> \
		<style> \
			body {background-color:blue;} \
			h1 {text-align:center;} \
			table, th, td { \
				border: 1px solid black; \
				border-style: inset; \
			} \
			th, td { \
				background-color: white; \
			} \
			table.center { \
				margin-left: auto; \
				margin-right: auto; \
			} \
		</style> \
		<script> \
			let speed = 0; \
			\
			function DecreaseSpeed() \
			{ \
				speed--; \
				if ( speed < 0) \
				{ \
					speed = 0; \
				} \
				document.getElementById(\"speed\").innerHTML = speed; \
			} \
			\
			function IncreaseSpeed() \
			{ \
				speed++; \
				if ( speed > 100 ) \
				{ \
					speed = 100; \
				} \
				document.getElementById(\"speed\").innerHTML = speed; \
			} \
		</script> \
	</head> \
	<body> \
		<h1>Track Loop Control</h1> \
		<hr /> \
		<p style=\"text-align:center;\">This page controls the speed of the locomotive</p> \
		<hr /> \
		<table class=\"center\"> \
			<tr style=\"text-align:center;\"> \
				<td><button type=\"button\" onclick=\"DecreaseSpeed()\">&#9660;</button></td> \
				<td id=\"speed\" style=\"width:50px\">0</td> \
				<td><button type=\"button\" onclick=\"IncreaseSpeed()\">&#9650;</button></td> \
			</tr> \
		</table> \
		<p style=\"text-align:center;\"> \
			<a href=\"index.html\", style=\"color:red;\">Exit</a> \
		</p> \
	</body> \
</html> \
"};
