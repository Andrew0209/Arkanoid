how to detect ball's reflection: <br>
Let fieldSize - pixel 'size' of the field <br>
Also, for simplisity, let X axis be coleniaar with robot's movement, Y axis starts with left border of the field. <br>
ballTarget.x = ball.point.x + (vx * (robot.point.y - ball.point.y) / vy); - this code calculate x'th position of the ball at the robol line<br>
to calcilate real ball's position we need co count reflections it will be count = ballTarget.x / fieldSize.x <br>
Let dist = ballTarget.x % fieldSize.x; <br>
Ball coordinates after reflections will be dist, if count is even and fieldSize.x - dist, if count is odd <br>
code: <br>
```
ballTarget.x = ball.point.x + (vx * (robot.point.y - ball.point.y) / vy);
int count = ballTarget.x / fieldSize.x;
int dist = ballTarget.x % fieldSize.x;
if (count % 2) ballTarget.x = fieldSize.x - dist;
else ballTarget.x = dist;
```
