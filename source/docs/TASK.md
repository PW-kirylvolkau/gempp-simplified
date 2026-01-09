# Laboratory Task Description

> Original message from Prof. Władysław Homenda

Dear All,

Summary of assumptions and procedure for the laboratory task:

1. The task can be carried out in teams of no more than four people in one of two versions:
   - a) **basic**, the subject of the task is graphs, the solution is rated on a scale up to 3.5,
   - b) **advanced**, the subject is multigraphs, and the solution is rated on a full scale up to 5.0.

   In the case of the advanced version, in the description below, the word graphs means multigraphs.

2. Input data will be saved in a format common to all teams, assuming data interchangeability between teams. These will be text files containing descriptions of two graphs:
   - the first line of the description of a first graph contains the number of vertices; this information is saved in one line of the file,
   - the following lines of the file contain rows of the adjacency matrix with elements separated by a space,
   - the first line of the description of a second graph contains the number of vertices; this information is saved in one line of the file,
   - the following lines of the file contain rows of the adjacency matrix with elements separated by a space,
   - additional data is optional and may be written in subsequent file lines.

   The adjacency matrix contains the number of edges between vertices (for graphs, it will be 0 or 1; for multigraphs, it will be 0 or a positive integer). For undirected graphs, the adjacency matrix will be symmetrical with regard to the main diagonal. Moreover, for example, information about the type of graph can be additionally written directly in lines after the adjacency matrix.

3. The solution to the task will be a report and a computer program compiled and run on computers in laboratory room 301 using Windows. The following concepts should be developed:
   - a) graph size, please use a reasonable definition consistent with the use of this concept,
   - b) a reasonable metric in the set of all graphs,
   - c) minimal extension of the second graph such that the extension includes a given number (one in the simplest case) of subgraphs that are isomorphic with the first graph.

   The report should include:
   - a) descriptions/definitions of the above concepts with justifications/evidence. Descriptions may be prepared based on literature (in which case the source should be cited and a short justification should be provided) or developed by the author (in such a case, a brief justification of the invented definition should be provided),
   - b) descriptions of algorithms for implementing concepts with justification of computational complexity. In the case of exponential complexity of the exact algorithm, an approximation algorithm with polynomial complexity should be developed,
   - c) results of computational tests with time characteristics ("calculation time" depending on the size of the task),
   - d) a short technical description of the program with detailed instructions for compiling and running the program,
   - e) conclusions.

4. The laboratory task can be carried out in the following mode:
   - a) formal, the rules are provided on the course website https://homenda.mini.pw.edu.pl/pages/21,
   - b) individual.

   In both cases, the deadline for submitting the solution to the task is the first week of December, i.e., December 5 at the latest. I encourage teams to turn in their solutions early.

5. Laboratory meetings will be carried out:
   - a) Fridays, 12:15-13:35 and 14:15-15:45, room 301,
   - b) Fridays, 18:30-20:00, Teams,

   The meeting will be closed:
   - if everyone who wants is served,
   - if no one shows up for the meeting within the first quarter.

**NOTE:** any deviations from the above assumptions should be agreed upon individually.

*Władysław Homenda*
