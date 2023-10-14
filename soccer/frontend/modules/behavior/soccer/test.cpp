void main() {
    DirectedCoorsd target_pos;

    // calculate the center of the goal
    float origin_x = (abs(goal_low_right.wcs_x - goal_low_left.wcs_x))/2;
    float origin_y = (abs(goal_low_right.wcs_y - goal_low_left.wcs_y))/2;

    // growth = (y_b - y_a) / (x_b - x_a)
    // calculate the line between origin and ball_rcs_pos
    // y = y1 + (y2-y1)/(x2-x1)*(x-x1)
    float line1_growth = (ball_wcs_pos.y - origin_y) / (ball_wcs_pos.x - origin_x);
    // b defined as: y_2 = x_1 * m + b
    // -b = x_1 * m - y_2
    // b = y_2 - x_1*m
    float line1_offset = origin_y - origin_x * line1_growth;



    float bot_x = bot_pos.coord.x;
    float bot_y = bot_pos.coord.y;

    // calculate the line orthogonal to the line between origin and ball_rcs_pos
    float line2_growth = (-1)/line1_growth;
    // insert bot pos and rearrange for offset
    //bot_y = line2_growth* bot_x + r;
    //-r + bot_y = line2_growht * bot_x;
    //-r = line2_growth * bot_x - bot_y;
    float line2_offset = bot_y - line2_growth*bot_x;

    // find point where both lines cross
    //line1_growth * target_x + line1_offset = line2_growth * target_x + line2_offset; || - line1_offset
    //rearrange for target_x
    //line1_growth * target_x = line2_growth * target_x + line2_offset - line1_offset; || / target_x
    //line1_growth = line2_growth + line2_offset / target_x - line1_offset / target_x; || - line2_growth
    //line1_growth - line2_growth = line2_offset / target_x - line1_offset / target_x; || * target_x
    //target_x * line1_growth - target_x * line2_growth = line2_offset - line1_offset; || exclude x
    // target_x * (line1_growth - line2_growth) = line2_offset - line1_offset;         || * (line1_growth-line2_growth)
    float target_x = (line2_offset - line1_offset)/(line1_growth - line2_growth);
    float target_y =  line2_growth * target_x + line2_offset;

    //find cross of line1 and the upper penalty bound
    float bound_top_growth = (goal_top_right.wcs_y - goal_top_left.wcs_y)/
                             (goal_top_right.wcs_x - goal_top_left.wcs_x);
    float bound_top_offset = (bound_top_growth * goal_top_right.wcs_x) *
                             (-1) + goal_top_right.wcs_y;

    float bound_top_x = (line1_offset - bound_top_offset)/(bound_top_growth -
                        line1_growth);
    float bound_top_y =  line1_growth * bound_top_x + line1_offset;

    //find cross of line1 and left bound

    float bound_left_growth = (goal_top_left.wcs_y - goal_low_left.wcs_y)/
                              (goal_top_left.wcs_x - goal_low_left.wcs_x);
    float bound_left_offset = (bound_left_growth * goal_top_left.wcs_x) *
                              (-1) + goal_top_left.wcs_y;

    float bound_left_x = (line1_offset - bound_left_offset)/
                         (bound_left_growth - line1_growth);
    float bound_left_y =  line1_growth * bound_left_x + line1_offset;

    // find cross of line2 and right bound

    float bound_right_growth = (goal_top_right.wcs_y - goal_low_right.wcs_y)/
                               (goal_top_right.wcs_x - goal_low_right.wcs_x);
    float bound_right_offset = (bound_right_growth * goal_top_right.wcs_x) *
                               (-1) + goal_top_right.wcs_y;

    float bound_right_x = (line1_offset - bound_right_offset)/
                          (bound_right_growth - line1_growth);
    float bound_right_y =  line1_growth * bound_right_x + line1_offset;



    // distance left and right will always be the same (?) so we only need to calculate one
    float distance_left = sqrt(pow(origin_x-bound_left_x,
                                   2) + pow(origin_y-bound_left_y, 2));
    float distance_top = sqrt(pow(bound_top_x-origin_x,
                                  2)+pow(bound_top_y-origin_y, 2));

    // if the shortest way lies between the penalty zone and the ball, go there
    // else move to the interception point on the penalty line

    if (distance_left == std::min(distance_left, distance_top)) {
        if (target_x <= std::max(bound_left_x, ball_wcs_pos.x)
                || target_x >= std::min(ball_wcs_pos.x, bound_left_x)) {
            target_pos = DirectedCoord(target_x, target_y, 0);
            return target_pos;
        }
        if (target_x <= std::max(bound_right_x, ball_wcs_pos.x)
                || target_x >= std::min(ball_wcs_pos.x, bound_right_x)) {
            target_pos = DirectedCoord(target_x, target_y, 0);
            return target_pos;
        } else {
            float distance_ball_left = sqrt(pow(bound_left_x -ball_wcs_pos.x,
                                                2)+pow(bound_left_y-ball_wcs_pos.y, 2));
            float distance_ball_right = sqrt(pow(bound_right_x-ball_wcs_pos.x,
                                                 2)+pow(bound_right_y-ball_wcs_pos.y, 2));
            if (distance_ball_left <= distance_ball_right) {
                target_pos = DirectedCoord(bound_left_x, bound_left_y, 0);
                return target_pos;
            } else {
                target_pos = DirectedCoord(bound_right_x, bound_right_y, 0);
                return target_pos;
            }
        }
    }


    if (target_x <= std::max(bound_top_x, ball_wcs_pos.x)
            || target_x >= std::min(ball_wcs_pos.x, bound_top_x)) {
        target_pos = DirectedCoord(target_x, target_y, 0);
        return target_pos;
    } else {
        target_pos = DirectedCoord(bound_top_x, bound_top_y, 0);
    }
    return target_pos;






}