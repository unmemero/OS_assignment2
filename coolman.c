#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>

void generate_pdf(const char *command) {
    // Create a dedicated directory in the home folder for generated files
    const char *base_dir = "/home/pills/manpages";
    mkdir(base_dir, 0755);

    // Define file paths within the dedicated directory
    char html_file[128], styled_html_file[128], pdf_file[128];
    char man_command[256], pdf_command[512];
    snprintf(html_file, sizeof(html_file), "%s/%s_manpage.html", base_dir, command);
    snprintf(styled_html_file, sizeof(styled_html_file), "%s/%s_manpage_styled.html", base_dir, command);
    snprintf(pdf_file, sizeof(pdf_file), "%s/%s_manpage.pdf", base_dir, command);

    // Step 1: Generate the HTML version of the man page using groff and aha
    snprintf(man_command, sizeof(man_command), "man %s | groff -Tascii -mandoc | aha > %s", command, html_file);
    printf("Running command: %s\n", man_command);
    if (system(man_command) != 0) {
        fprintf(stderr, "Failed to generate HTML for man page.\n");
        return;
    }

    // Check if HTML file was created
    if (access(html_file, F_OK) == -1) {
        fprintf(stderr, "Error: HTML file %s was not created.\n", html_file);
        return;
    } else {
        printf("HTML file created successfully: %s\n", html_file);
    }

    // Step 2: Add CSS for syntax highlighting and markdown-like formatting
    FILE *html_fp = fopen(html_file, "r");
    FILE *styled_html_fp = fopen(styled_html_file, "w");
    if (!html_fp || !styled_html_fp) {
        fprintf(stderr, "Error opening files for reading/writing.\n");
        if (html_fp) fclose(html_fp);
        return;
    }

    // Add CSS style for syntax highlighting and markdown-like structure
    fprintf(styled_html_fp,
        "<!DOCTYPE html><html><head><style>"
        "body { font-family: monospace; background: #f4f4f4; color: #333; line-height: 1.5; margin: 20px; }"
        "h1 { color: #2e6da4; font-size: 24px; border-bottom: 2px solid #2e6da4; padding-bottom: 5px; }"
        "h2 { color: #31708f; font-size: 20px; border-bottom: 1px solid #31708f; padding-bottom: 3px; }"
        "pre { background: #333; color: #f8f8f2; padding: 10px; border-radius: 5px; overflow: auto; }"
        "code { background: #f5f5f5; padding: 3px 5px; border-radius: 3px; color: #d14; }"
        "a { color: #31708f; text-decoration: none; } a:hover { text-decoration: underline; }"
        "</style></head><body>");

    // Copy the HTML content to the styled HTML file
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), html_fp) != NULL) {
        fputs(buffer, styled_html_fp);
    }

    // Close HTML structure
    fprintf(styled_html_fp, "</body></html>");
    fclose(html_fp);
    fclose(styled_html_fp);

    // Check if styled HTML file was created
    if (access(styled_html_file, F_OK) == -1) {
        fprintf(stderr, "Error: Styled HTML file %s was not created.\n", styled_html_file);
        return;
    } else {
        printf("Styled HTML file created successfully: %s\n", styled_html_file);
    }

    // Step 3: Convert styled HTML to PDF with wkhtmltopdf
    snprintf(pdf_command, sizeof(pdf_command), "wkhtmltopdf %s %s 2>&1", styled_html_file, pdf_file);
    printf("Running command: %s\n", pdf_command);

    FILE *cmd_output = popen(pdf_command, "r");
    if (cmd_output == NULL) {
        fprintf(stderr, "Failed to run wkhtmltopdf command.\n");
        unlink(html_file);
        unlink(styled_html_file);
        return;
    }

    // Print wkhtmltopdf output for debugging
    char cmd_output_buffer[1024];
    while (fgets(cmd_output_buffer, sizeof(cmd_output_buffer), cmd_output) != NULL) {
        printf("%s", cmd_output_buffer);
    }
    pclose(cmd_output);

    // Check if PDF file was created
    if (access(pdf_file, F_OK) == -1) {
        fprintf(stderr, "Error: PDF file %s was not created.\n", pdf_file);
        unlink(html_file);
        unlink(styled_html_file);
        return;
    } else {
        printf("PDF file created successfully: %s\n", pdf_file);
    }

    // Step 4: Open the PDF with xdg-open
    if (fork() == 0) {
        execlp("xdg-open", "xdg-open", pdf_file, NULL);
        perror("Failed to open PDF");
        exit(EXIT_FAILURE);
    }

    // Wait for the PDF viewer to close and delete temporary files
    wait(NULL);
    /*
    unlink(html_file);         // Remove original HTML file
    unlink(styled_html_file);  // Remove styled HTML file
    unlink(pdf_file);          // Remove PDF after viewing88866714
    */
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <command>\n", argv[0]);
        return EXIT_FAILURE;
    }

    generate_pdf(argv[1]);
    return EXIT_SUCCESS;
}
