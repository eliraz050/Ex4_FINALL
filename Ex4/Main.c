#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h> //uncomment this block to check for heap memory allocation leaks.
 //Read https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2019
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LINESIZE 1023

typedef struct StudentCourseGrade
{
	char courseName[35];
	int grade;
}StudentCourseGrade;

typedef struct Student
{
	char name[35];
	StudentCourseGrade* grades; //dynamic array of courses
	int numberOfCourses;
}Student;


//Part A
void countStudentsAndCourses(const char* fileName, int** coursesPerStudent, int* numberOfStudents);
int countPipes(const char* lineBuffer, int maxCount);
char*** makeStudentArrayFromFile(const char* fileName, int** coursesPerStudent, int* numberOfStudents);
void printStudentArray(const char* const* const* students, const int* coursesPerStudent, int numberOfStudents);
void factorGivenCourse(char** const* students, const int* coursesPerStudent, int numberOfStudents, const char* courseName, int factor);
void studentsToFile(char*** students, int* coursesPerStudent, int numberOfStudents);

//Part B
Student* transformStudentArray(char*** students, const int* coursesPerStudent, int numberOfStudents);
void writeToBinFile(const char* fileName, Student* students, int numberOfStudents);
Student* readFromBinFile(const char* fileName);

int main()
{
	//Part A
	int* coursesPerStudent = NULL;
	int numberOfStudents = 0;
	char*** students = makeStudentArrayFromFile("studentList.txt", &coursesPerStudent, &numberOfStudents);
	printStudentArray(students, coursesPerStudent, numberOfStudents);
	factorGivenCourse(students, coursesPerStudent, numberOfStudents, "Linear Algebra", +5);
	printStudentArray(students, coursesPerStudent, numberOfStudents);
	//studentsToFile(students, coursesPerStudent, numberOfStudents); //this frees all memory. Part B fails if this line runs. uncomment for testing (and comment out Part B)

	//Part B
	Student* transformedStudents = transformStudentArray(students, coursesPerStudent, numberOfStudents);
	writeToBinFile("students.bin", transformedStudents, numberOfStudents);
	Student* testReadStudents = readFromBinFile("students.bin");

	//free all arrays of struct Student
	for (int i = 0; i < numberOfStudents; i++)
	{
		free(transformedStudents[i].grades);
		free(testReadStudents[i].grades);
	}

	free(transformedStudents);
	free(testReadStudents);

	_CrtDumpMemoryLeaks(); //uncomment this block to check for heap memory allocation leaks.
	 //Read https://docs.microsoft.com/en-us/visualstudio/debugger/finding-memory-leaks-using-the-crt-library?view=vs-2019

	return 0;
}

void countStudentsAndCourses(const char* fileName, int** coursesPerStudent, int* numberOfStudents)
{
	FILE* file = fopen(fileName, "rt"); //open file

	if (file == NULL) { //check that open was sucessful
		printf("Unable to open file\n");
		exit(1);
	}

	int counter = 0; // count the number of lines in the file (aka number of students)
	char s[1023];
	while (fgets(s, LINESIZE, file) != NULL) {
		counter++;
	}
	*numberOfStudents = counter;

	*coursesPerStudent = (int*)malloc(counter * sizeof(int*)); //allocate memorey for courses by number of students
	if (coursesPerStudent == NULL) {
		printf("memory allocation failed");
		exit(1);
	}


	rewind(file); //set the file position to the beginning of the file



	for (int i = 0; i < counter; i++) {  // count and put in array the number of courses for each student
		int num;
		fgets(s, 1023, file);
		num = countPipes(s, LINESIZE);
		(* coursesPerStudent)[i] = num;
	}
	fclose(file);
}

int countPipes(const char* lineBuffer, int maxCount)
{
	if (lineBuffer == NULL) return -1; // if no sentence provided return -1
	if (maxCount <= 0) return 0; // if no reall sentence size provided return 0
	int counter = 0;
	for (int i = 0; i < maxCount; i++) { // count pipe lines
		if (lineBuffer[i] == '|') counter++;
		if (lineBuffer[i] == '\0') break; // stop when end of line reached
	}
	return counter; // return number of pipe lines
}

char*** makeStudentArrayFromFile(const char* fileName, int** coursesPerStudent, int* numberOfStudents)
{
	FILE* file = fopen(fileName, "rt"); // open file to read

	if (file == NULL) { //check that open was sucessful
		printf("Unable to open file\n");
		exit(1);
	}

	countStudentsAndCourses(fileName, coursesPerStudent, numberOfStudents); // update coursesPerstudent and numOfStudents

	char*** array = (char***)malloc(*numberOfStudents * sizeof(char***)); // create students array

	if (array == NULL) {  //check memory allocation sucessfull
		printf("memory alloaction failed");
		exit(1);
	}

	char* s = (char*)malloc(LINESIZE * sizeof(char*)); // sentence pointer 

	if (s == NULL) { //check memory allocation sucessfull
		printf("memory alloaction failed");
		exit(1);
	}

	char* a = s; // temp pointer to free sentence later
	int numOfstrings; // number of strings to divide the sentence to
	char* delimiter; // delimiter will change from | to ,
	int counter = 0; // counter to choose what deliimter needed

	for (int i = 0; i < *numberOfStudents; i++) { // loop to run on all students 
		fgets(s, LINESIZE, file); // put one line from the file in s
		numOfstrings = (*coursesPerStudent)[i] * 2 + 1; // number of strings needed is number of courses times 2 (every course has a grade) plus the name of the student
		array[i] = (char**)malloc(numOfstrings * sizeof(char**)); 

		if (array[i] == NULL) { //check memory allocation sucessfull
			printf("memory alloaction failed");
			exit(1);
		}

		for (int j = 0; j < numOfstrings; j++) {  // loop for each string needed 
			if (counter % 2 == 0) {
				delimiter = "|";
			}
			else {
				delimiter = ",";
			}
			char* word;
			word = strtok(s, delimiter);
			s = s + strlen(word) + 1; // advance pointer to next word
			array[i][j] = (char*)malloc(strlen(word) * sizeof(char*)); // allocate memory in student array

			if (array[i][j] == NULL) { //check memory allocation sucessfull
				printf("memory alloaction failed");
				exit(1);
			}

			strcpy(array[i][j], word); 
			counter++;

		}
		counter = 0; // initalize counter
	}
	free(a); // free line memory
	fclose(file); // close file
	return array;
}

void factorGivenCourse(char** const* students, const int* coursesPerStudent, int numberOfStudents, const char* courseName, int factor)
{
	if (factor > 20 || factor < (-20)) return; // if factor is too high or low do nothing
	int grade; // current grade
	char gradeWithFactor[4]; // factor to give
	for (int i = 0; i < numberOfStudents; i++) { // loop all students

		for (int j = 0; j < coursesPerStudent[i]; j++) { // loop all courses

			if (j % 2 == 0) { //skip courses grades 

				if (strcmp(students[i][j + 1], courseName) == 0) { // find course to give factor to
					grade = atoi(students[i][j + 2]); // make current grade char to int
					grade = grade + factor;  // give factor
					if (grade > 100) grade = 100; 
					if (grade < 0) grade = 0;
					_itoa(grade, gradeWithFactor, 10); // make int grade to char
					strcpy(students[i][j + 2], gradeWithFactor); // put grade back in students array
					break; // no two grades to one course per student so break loop
				}

			}

		}

	}
}

void printStudentArray(const char* const* const* students, const int* coursesPerStudent, int numberOfStudents)
{
	for (int i = 0; i < numberOfStudents; i++)
	{
		printf("name: %s\n*********\n", students[i][0]);
		for (int j = 1; j <= 2 * coursesPerStudent[i]; j += 2)
		{
			printf("course: %s\n", students[i][j]);
			printf("grade: %s\n", students[i][j + 1]);
		}
		printf("\n");
	}
}

void studentsToFile(char*** students, int* coursesPerStudent, int numberOfStudents)
{
	FILE* file = fopen("studentList.txt", "wt"); // create student file

	int counter = 0; // counter to choose to put | or , 
	char delimiter[2]; // delimiter string including \0 
	char line[1022] = ""; // the \n is added alone by fputs so only 1022 allowed
	for (int i = 0; i < numberOfStudents; i++) { // loop for new line for each student
		for (int j = 0; j < coursesPerStudent[i]  * 2 + 1; j++) { // loop to put strings in one line to put to file
			if (counter % 2 == 0) {
				strcpy(delimiter, "|");
			}
			if (counter % 2 != 0) {
				strcpy(delimiter, ",");
			}
			if (counter == coursesPerStudent[i] * 2) { //case end of line
				strcpy(delimiter, "");
			}
			strcat(line, students[i][j]); 
			strcat(line, delimiter);
			counter++;
			free(students[i][j]);
		}
		fputs(line, file);
		strcpy(line, ""); // initialize line
		counter = 0; // initalize delimiter counter
		free(students[i]);
	}
	fclose(file); //close file so it can update
	free(coursesPerStudent);
	free(students);
}

void writeToBinFile(const char* fileName, Student* students, int numberOfStudents)
{
	//create pointer to file
	FILE* bfile;

	//open new binary file in write mode, exits if file pointer returns null
	if ((bfile = fopen(fileName, "wb")) == NULL) {
		printf("Error! opening bin file");
		exit(1);
	}

	//add number of students to start of file
	fwrite(&numberOfStudents, sizeof(int), 1, bfile);

	for (int i = 0; i < numberOfStudents; i++) {

		//write name of student
		fwrite(students[i].name, 35 * sizeof(char), 1, bfile);

		//write number of courses of student
		fwrite(&students[i].numberOfCourses, sizeof(int), 1, bfile);

		//write student's courses and grades
		for (int j = 0; j < students[i].numberOfCourses; j++) {
			fwrite(students[i].grades[j].courseName, 35, 1, bfile);
			fwrite(&students[i].grades[j].grade, 4, 1, bfile);
		}
	}

	//close file
	fclose(bfile);
}

Student* readFromBinFile(const char* fileName)
{
	int numStudents;

	//create pointer to file
	FILE* bfile;

	//open file, if action fails exit program

	if ((bfile = fopen(fileName, "rb")) == NULL) {
		printf("Error! opening bin file");
		exit(1);
	}

	//read number of students in file
	fread(&numStudents, sizeof(int), 1, bfile);

	//allocate struct array to number of students
	Student* students = (Student*)malloc(numStudents * sizeof(Student));

	//verify list was allocated succesfully
	if (students == NULL) {
		printf("memory allocation failed");
		exit(1);
	}

	//fill in information for each student
	for (int i = 0; i < numStudents; i++) {

		//save student name in struct
		fread(students[i].name, 35 * sizeof(char), 1, bfile);

		//save student's number of courses
		fread(&students[i].numberOfCourses, sizeof(int), 1, bfile);

		//allocate struct array to number of course
		students[i].grades = (StudentCourseGrade*)malloc((students[i].numberOfCourses) * sizeof(StudentCourseGrade));

		//save information for each course
		for (int j = 0; j < students[i].numberOfCourses; j++) {
			fread(students[i].grades[j].courseName, 35, 1, bfile);
			fread(&students[i].grades[j].grade, 4, 1, bfile);
		}
	}

	//close file
	fclose(bfile);

	//return struct array
	return (students);
}

Student* transformStudentArray(char*** students, const int* coursesPerStudent, int numberOfStudents)
{
	//dynamically allocate student struct list
	Student* studentList = (Student*)malloc(numberOfStudents * sizeof(Student));

	//verify list was allocated succesfully
	if (studentList == NULL) {
		printf("memory allocation failed");
		exit(1);
	}

	//add each student in array to student struct list
	for (int i = 0; i < numberOfStudents; i++) {

		//copy student name
		strcpy(studentList[i].name, students[i][0]);

		//copy student's number of courses
		studentList[i].numberOfCourses = coursesPerStudent[i];

		//dynamically allocate student course grade struct list for each student
		studentList[i].grades = (StudentCourseGrade*)malloc(coursesPerStudent[i] * sizeof(StudentCourseGrade));

		for (int j = 1, k = 0; j <= 2 * studentList[i].numberOfCourses; j += 2, k++) {
			//copy course name for every course
			strcpy(studentList[i].grades[k].courseName, students[i][j]);
			//copy course grade for every course after converting from string to integer
			studentList[i].grades[k].grade = atoi(students[i][j + 1]);
		}
	}

	return (studentList);
}

void printStudentStruct(Student* students, int numberOfStudents) {
	for (int i = 0; i < numberOfStudents; i++)
	{
		printf("name: %s\n", students[i].name);
		printf("courses per student %d\n", students[i].numberOfCourses);

		for (int j = 0; j < students[i].numberOfCourses; j++) {
			printf("course: %s\n", students[i].grades[j].courseName);
			printf("grade: %d\n", students[i].grades[j].grade);
		}

		printf("\n");
	}
}
