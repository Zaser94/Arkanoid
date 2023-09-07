-----------------------------------------------------------------------------------------------
Arkanoid like game made in two days using C++ and SDL2 for rendering the graphics.
-----------------------------------------------------------------------------------------------

You can double click the executable called Arkanoid.exe 
					
					or
					
You can open the visual studio project with Visual Studio 2022 (Otherwise it wouldn't compile)

Inside Visual Studio 2022, files are showed ordered.
-----------------------------------------------------------------------------------------------

Controls:

- Left and right arrows -> move the slide bar.

- Mouse left click ->  throw the ball in the desired direction.

- Mouse Right Click -> If you lose the ball you can recover it. (Known issue)  

Features:

- Blocks have 1-4 lives depending on their color.

- Blocks slowly move downwards at a constant speed.

- After 10 seconds 3 different power-ups and 3 different debuffs will start to appear. You can catch them or avoid them moving the slide bar.

Win condition: 

- Break all blocks.

Lose conditions: 

- If the ball reach the bottom you will lose 1 live. If you lose 3 lives -> game over.

- If any block reach the bottom or touch the player -> game over.