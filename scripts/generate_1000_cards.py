#!/usr/bin/env python3
"""Generate a test JSON file with 1000 cards for performance testing."""

import json
import uuid
import random
from datetime import datetime, timedelta

# Sample topics and content templates
topics = [
    ("C++ Basics", "C++ is a general-purpose programming language. Key concepts include variables, data types, control flow, and functions."),
    ("Object-Oriented Programming", "OOP principles include encapsulation, inheritance, polymorphism, and abstraction."),
    ("Data Structures", "Common data structures include arrays, linked lists, stacks, queues, trees, and hash tables."),
    ("Algorithms", "Algorithm design techniques include divide and conquer, dynamic programming, greedy algorithms, and backtracking."),
    ("Design Patterns", "Software design patterns provide reusable solutions to commonly occurring problems in software design."),
    ("Operating Systems", "Operating system concepts include processes, threads, memory management, file systems, and I/O."),
    ("Computer Networks", "Networking fundamentals cover TCP/IP, HTTP, DNS, routing protocols, and network security."),
    ("Databases", "Database concepts include relational models, SQL, indexing, transactions, and normalization."),
    ("Machine Learning", "Machine learning involves supervised learning, unsupervised learning, reinforcement learning, and neural networks."),
    ("Software Engineering", "Software engineering practices include version control, testing, CI/CD, code review, and agile methodology."),
    ("Cryptography", "Cryptography covers symmetric encryption, asymmetric encryption, hashing, digital signatures, and PKI."),
    ("Compilers", "Compiler design involves lexical analysis, parsing, semantic analysis, optimization, and code generation."),
    ("Computer Architecture", "Computer architecture includes CPU design, memory hierarchy, pipelining, and parallel processing."),
    ("Functional Programming", "Functional programming emphasizes immutability, pure functions, higher-order functions, and recursion."),
    ("Concurrency", "Concurrency concepts include threads, mutexes, semaphores, deadlock, and synchronization primitives."),
    ("Git", "Git is a distributed version control system that tracks changes in source code during software development."),
    ("Docker", "Docker provides containerization for applications, enabling consistent deployment across different environments."),
    ("Linux", "Linux is an open-source Unix-like operating system kernel used in servers, embedded systems, and mobile devices."),
    ("Python", "Python is a high-level programming language known for its readability and extensive standard library."),
    ("Rust", "Rust is a systems programming language focused on safety, concurrency, and performance without garbage collection."),
    ("TypeScript", "TypeScript is a typed superset of JavaScript that adds optional static typing and class-based OOP."),
    ("Go", "Go is a statically typed language designed for simplicity, efficiency, and built-in concurrency support."),
    ("Web Development", "Web development involves HTML, CSS, JavaScript, frontend frameworks, and backend technologies."),
    ("API Design", "RESTful API design principles include resource-based URLs, HTTP methods, status codes, and pagination."),
    ("Testing", "Software testing includes unit testing, integration testing, end-to-end testing, and test-driven development."),
    ("Cloud Computing", "Cloud computing provides on-demand computing resources including IaaS, PaaS, and SaaS models."),
    ("GraphQL", "GraphQL is a query language for APIs that allows clients to request exactly the data they need."),
    ("Microservices", "Microservices architecture decomposes applications into small, independent services with well-defined interfaces."),
    ("Kubernetes", "Kubernetes automates deployment, scaling, and management of containerized applications."),
    ("Redis", "Redis is an in-memory data structure store used as database, cache, and message broker."),
]

# Content extensions for variety
extensions = [
    "This topic is fundamental to modern software development and is widely used in industry.",
    "Understanding this concept is essential for building scalable and maintainable software systems.",
    "This area has seen significant advances in recent years with new tools and frameworks.",
    "Mastery of this subject can greatly improve code quality and developer productivity.",
    "This is a key skill for senior software engineers and technical leaders.",
    "Research in this area continues to evolve with practical applications in production systems.",
    "Many companies rely on this technology for their core business operations.",
    "This concept bridges the gap between theoretical computer science and practical engineering.",
    "Hands-on experience with this topic is highly valued in technical interviews.",
    "This technology stack is commonly used in large-scale distributed systems.",
]

# LaTeX formulas for some cards
latex_formulas = [
    r"The time complexity is $O(n \log n)$ on average.",
    r"The formula is $E = mc^2$, where $E$ is energy and $m$ is mass.",
    r"Bayes' theorem: $P(A|B) = \frac{P(B|A)P(A)}{P(B)}$.",
    r"The sum of an arithmetic series: $S = \frac{n(a_1 + a_n)}{2}$.",
    r"Gradient descent update: $\theta_{t+1} = \theta_t - \alpha \nabla J(\theta_t)$.",
    r"Big-O comparison: $O(1) < O(\log n) < O(n) < O(n \log n) < O(n^2)$.",
    r"Euler's identity: $e^{i\pi} + 1 = 0$.",
    r"The quadratic formula: $x = \frac{-b \pm \sqrt{b^2 - 4ac}}{2a}$.",
]

def generate_card(index):
    topic = random.choice(topics)
    extension = random.choice(extensions)
    include_latex = random.random() < 0.2  # 20% cards have LaTeX

    content = topic[1] + " " + extension
    if include_latex:
        content = random.choice(latex_formulas) + " " + content

    # Add some uniqueness
    card_num = index + 1
    title = f"{topic[0]} #{card_num}"

    # Spread creation times over the past year
    days_ago = random.randint(0, 365)
    hours_offset = random.randint(0, 23)
    minutes_offset = random.randint(0, 59)
    created = datetime(2026, 2, 26) - timedelta(days=days_ago, hours=hours_offset, minutes=minutes_offset)

    return {
        "id": str(uuid.uuid4()),
        "title": title,
        "content": content,
        "created": created.strftime("%Y-%m-%dT%H:%M:%S")
    }


def main():
    cards = [generate_card(i) for i in range(1000)]

    now = datetime.now().strftime("%Y-%m-%dT%H:%M:%S")

    # --- CardManager format (for unit test) ---
    data_v1 = {
        "version": "1.0",
        "created": now,
        "description": "Performance test data with 1000 cards",
        "cards": cards
    }
    output_path = "cards_1000_test.json"
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(data_v1, f, indent=2, ensure_ascii=False)
    print(f"Generated {len(cards)} cards to {output_path}")

    # --- Deck v2.0 format (for the actual app) ---
    import os
    deck_dir = os.path.expanduser("~/.knowledgecardgame/decks")
    os.makedirs(deck_dir, exist_ok=True)

    deck_data = {
        "version": "2.0",
        "name": "Performance Test 1000",
        "description": "1000 cards for performance testing",
        "createdAt": now,
        "lastModified": now,
        "cards": cards
    }
    # Match DeckRepository::deckNameToFileName logic:
    # lowercase, spaces->underscores, '+'->'p', remove non [a-z0-9_p]
    deck_filename = "performance_test_1000.json"
    deck_path = os.path.join(deck_dir, deck_filename)
    with open(deck_path, "w", encoding="utf-8") as f:
        json.dump(deck_data, f, indent=2, ensure_ascii=False)
    print(f"Installed deck to {deck_path}")
    print(f"Launch the app and select 'Performance Test 1000' deck to test.")


if __name__ == "__main__":
    main()
