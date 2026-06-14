#ifndef LINE_SEARCHING_H
#define LINE_SEARCHING_H

/**
 * Run the line search algorithm.
 * Swirls the robot from left to right till the line is found.
 */
void line_searching_run();

/**
 * Reset the line searching handler.
 * If not called the robot will start to swirl with full amplitude at the next
 * start.
 */
void line_searching_reset();

#endif // !LINE_SEARCHING_H
