//Scoring system, assumes the user answered correctly, linear score decreasing
//answer_time = the time it took for the user to answer
//total_time = the total time given to the user
int score(int answer_time, int total_time)
{
  return 500 + (total_time-answer_time)/total_time;
}
//Scoring system, assumes the user answered correctly, linear score decrease with a user-specified buffer
//answer_time = the time it took for the user to answer
//total_time = the total time given to the user
//buffer = the amount of time before the score should start decreasing
int buffered_score(int answer_time, int total_time, int buffer)
{
  if(answer_time <= buffer)
    {
      return 1000;
    }
  return 500 + (total_time-answer_time+buffer)/total_time;
}

int get_score(int answer_time, int total_time, char correct_ans, char user_ans)
{
  if(correct_ans == user_ans)
    {
      return 500 + (total_time-answer_time)/total_time;
    }
  else
    return 0;    
}

int get_buffered_score(int answer_time, int total_time, int buffer, char correct_ans, char user_ans)
{
  if(correct_ans == user_ans)
    {
      if(answer_time <= buffer)
        {
          return 1000;
        }
      return 500 + (total_time-answer_time+buffer)/total_time;
    }
  else
    return 0;
    
}
