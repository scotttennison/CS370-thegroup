-- CS370 Auction Site Database Schema
-- Created by: scotttennison

CREATE DATABASE IF NOT EXISTS auction_site;
USE auction_site;

-- Users table (foundation)
CREATE TABLE users (
    user_id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(320),
    email VARCHAR(320) NOT NULL UNIQUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    first_name VARCHAR(64) NOT NULL,
    last_name VARCHAR(64) NOT NULL,
    phone_num VARCHAR(64),
    is_active BOOL NOT NULL,
    password_hash VARCHAR(255) NOT NULL
);

-- Categories table (foundation)
CREATE TABLE categories (
    category_id INT AUTO_INCREMENT PRIMARY KEY,
    parent_category_id INT,
    category_name VARCHAR(128) NOT NULL,
    FOREIGN KEY (parent_category_id) REFERENCES categories(category_id)
);

-- Address table (depends on users)
CREATE TABLE address (
    address_id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    street VARCHAR(128) NOT NULL,
    city VARCHAR(128) NOT NULL,
    state VARCHAR(128) NOT NULL,
    zipcode VARCHAR(20) NOT NULL,
    county VARCHAR(128) NOT NULL,
    FOREIGN KEY (user_id) REFERENCES users(user_id)
);

-- Listing table (depends on users, categories)
CREATE TABLE listing (
    listing_id INT AUTO_INCREMENT PRIMARY KEY,
    category_id INT,
    seller_id INT NOT NULL,
    title VARCHAR(128) NOT NULL,
    description TEXT,
    starting_price FLOAT NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    scheduled_end_time TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES categories(category_id),
    FOREIGN KEY (seller_id) REFERENCES users(user_id)
);

-- Auctions table (depends on listing)
CREATE TABLE auctions (
    auction_id INT AUTO_INCREMENT PRIMARY KEY,
    listing_id INT NOT NULL,
    start_time TIMESTAMP,
    end_time TIMESTAMP,
    auction_status BOOL,
    FOREIGN KEY (listing_id) REFERENCES listing(listing_id)
);

-- Bids table (depends on auctions, users)
CREATE TABLE bids (
    bid_id INT AUTO_INCREMENT PRIMARY KEY,
    auction_id INT NOT NULL,
    bidder_id INT NOT NULL,
    bid_amount FLOAT NOT NULL,
    bid_time TIMESTAMP NOT NULL,
    FOREIGN KEY (auction_id) REFERENCES auctions(auction_id),
    FOREIGN KEY (bidder_id) REFERENCES users(user_id)
);

-- Transactions table (depends on auctions, users)
CREATE TABLE transactions (
    transaction_id INT AUTO_INCREMENT PRIMARY KEY,
    auction_id INT NOT NULL,
    buyer_id INT NOT NULL,
    transaction_date TIMESTAMP NOT NULL,
    transaction_status BOOL NOT NULL,
    FOREIGN KEY (auction_id) REFERENCES auctions(auction_id),
    FOREIGN KEY (buyer_id) REFERENCES users(user_id)
);

-- Payments table (depends on transactions)
CREATE TABLE payments (
    payment_id INT AUTO_INCREMENT PRIMARY KEY,
    transaction_id INT NOT NULL,
    amount FLOAT NOT NULL,
    payment_date TIMESTAMP NOT NULL,
    payment_status BOOL,
    payment_method VARCHAR(64),
    FOREIGN KEY (transaction_id) REFERENCES transactions(transaction_id)
);

-- Shipping table (depends on transactions, address)
CREATE TABLE shipping (
    shipping_id INT AUTO_INCREMENT PRIMARY KEY,
    transaction_id INT NOT NULL,
    address_id INT NOT NULL,
    carrier VARCHAR(128) NOT NULL,
    tracking_number VARCHAR(128) NOT NULL,
    shipped_date TIMESTAMP NOT NULL,
    delivery_status BOOL,
    FOREIGN KEY (transaction_id) REFERENCES transactions(transaction_id),
    FOREIGN KEY (address_id) REFERENCES address(address_id)
);
