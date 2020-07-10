#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include <ctype.h>




/* Return the offset of the first newline in text or the length of
   text if there's no newline */
static int newline_offset(const char *text)
{
  const char *newline = strchr(text, '\n');
  if(!newline)
    return strlen(text);
  else
    return (int)(newline - text);
}



int main (){

  char ch;
  //char file_header[128];
  size_t i;
 json_t *root;
 json_error_t error;
    printf("Opening questions file\n");
    root = json_load_file("./questions_2.json", 0, &error);

    if(!root)
      {
        fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
        return 1;
      }
    json_t *questions;
    questions = json_object_get(root, "results");
    if(!json_is_array(questions))
      {
        fprintf(stderr, "error: questions is not an array\n");
        json_decref(questions);
        return 1;
      }
  // We need to start reading the contents here and delimiting as we do so


    for(i = 0; i < json_array_size(questions); i++)
      {
        json_t *data, *category, *type, *difficulty, *question, *correct_answer, *incorrect_answers;
        const char *question_text;
        const char *category_text;
        const char *type_text;
        const char *difficulty_text;
        const char *correct_answer_text;
        const char *incorrect_answers_text;

        data = json_array_get(questions, i);
        if(!json_is_object(data))
          {
            fprintf(stderr, "error: commit data %lu is not an object\n", i + 1);
            json_decref(root);
            return 1;
          }

        category = json_object_get(data, "category");
        if(!json_is_string(category))
          {
            fprintf(stderr, "error: commit %lu: category is not a string\n", i + 1);
            json_decref(root);
            return 1;
          }

        type = json_object_get(data, "type");
        if(!json_is_string(type))
          {
            fprintf(stderr, "error: commit %lu: type is not an object\n", i + 1);
            json_decref(root);
            return 1;
          }

        difficulty = json_object_get(data, "difficulty");
        if(!json_is_string(difficulty))
          {
            fprintf(stderr, "error: commit %lu: difficulty is not a string\n", i + 1);
            json_decref(root);
            return 1;
          }


        question = json_object_get(data, "question");
        if(!json_is_string(question))
          {
            fprintf(stderr, "error: commit %lu: question is not a string\n", i + 1);
            json_decref(root);
            return 1;
          }


        correct_answer = json_object_get(data, "correct_answer");
        if(!json_is_string(correct_answer))
          {
            fprintf(stderr, "error: commit %lu: correct_answer is not a string\n", i + 1);
            json_decref(root);
            return 1;
          }


        incorrect_answers = json_object_get(data, "incorrect_answers");
        if(!json_is_array(incorrect_answers))
          {
            fprintf(stderr, "error: commit %lu: incorrect_answers is not a string\n", i + 1);
            json_decref(root);
            return 1;
          }

        
        question_text = json_string_value(question);
        type_text = json_string_value(type);
        difficulty_text = json_string_value(difficulty);
        category_text = json_string_value(category);
        correct_answer_text = json_string_value(correct_answer);
        incorrect_answers_text = json_string_value(incorrect_answers);

        if(ispunct(question_text)){
          question_text.erase();
        }
        
        printf("Category: %s\n Difficulty: %s\n %.*s\n",
               category_text, difficulty_text,
               newline_offset(question_text),
               question_text);
      }
    
}
